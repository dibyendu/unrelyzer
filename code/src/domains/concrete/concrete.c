#include "concrete.h"

int N_iterations, first_program_point;
bool is_solution_fixed;

ConcreteState *init_concrete_state() {
  ConcreteState *state = (ConcreteState *) calloc(1, sizeof(ConcreteState));
  state->probability = 1.0;
  return state;
}

void free_concrete_state(ConcreteState *state) {
  void _free(IntSet *set) {
    if (!set) return;
    _free(set->left);
    _free(set->right);
    free(set);
  }

  if (state) {
    _free(state->value_set);
    if (state->component_states) {
      int i;
      for (i = 0; i < N_variables; ++i) {
        if (state->component_states[i]) {
          _free(state->component_states[i]->value_set);
          free(state->component_states[i]);
        }
      }
      free(state->component_states);
    }
    free(state);
  }
}

void insert_into_concrete_state_value_set(ConcreteState *state, int value) {
  if (state) {
    bool key_exists = false;
    state->value_set = avl_tree_insert(state->value_set, value, &key_exists);
    state->number_of_values += key_exists ? 0 : 1;
  }
}

static int initialize_first_program_point(bool initialize_all_uninitialized_variables_with_all_values_from_the_range) {
  int i, j;
  empty_state_status = (bool *) calloc(N_lines + 1, sizeof(bool));
  data_flow_graph = (bool **) calloc(N_lines + 1, sizeof(bool *));  
  for (i = 0; i <= N_lines; i++) {
    data_flow_graph[i] = (bool *) calloc(N_lines + 1, sizeof(bool));
    for (j = 0; j <= N_lines; j++)
      data_flow_graph[i][j] = data_flow_matrix[i] ? (data_flow_matrix[i][j] ? true : false) : false;
  }
  for (i = 1; i <= N_lines; i++) {
    if (data_flow_matrix[i]) {
      empty_state_status[i] = true;
      for (j = 0; j < N_variables; ++j) {
        if (!symbol_table[symbol_table_indices[j]].concrete)
          symbol_table[symbol_table_indices[j]].concrete = calloc(N_lines + 1, sizeof(ConcreteState *));
        if (initialize_all_uninitialized_variables_with_all_values_from_the_range) {
          symbol_table[symbol_table_indices[j]].concrete[i] = calloc(1, sizeof(ConcreteState));
          ((ConcreteState *) symbol_table[symbol_table_indices[j]].concrete[i])->probability = 1.0;
          int k;
          for (k = MININT; k <= MAXINT; ++k)
            insert_into_concrete_state_value_set((ConcreteState *) symbol_table[symbol_table_indices[j]].concrete[i], k);
        }
      }
      break;
    }
  }
  return i;
}

static int number_of_all_tuples_of_unique_variables(int program_point, Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  int _number_of_all_tuples_of_unique_variables(int program_point, Ast *node) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int index = get_symbol_table_index(node->token);
        if (visited[index]) return 1;
        visited[index] = true;
        return symbol_table[index].concrete[program_point] ?
               (((ConcreteState *) symbol_table[index].concrete[program_point])->number_of_values ?
               ((ConcreteState *) symbol_table[index].concrete[program_point])->number_of_values : 0) :
               0;
      }
      return 1;
    }
    int i = node->number_of_children, product = 1;
    while (i) product *= _number_of_all_tuples_of_unique_variables(program_point, node->children[--i]);
    return product;
  }

  return _number_of_all_tuples_of_unique_variables(program_point, node);
}

static void generate_all_possible_unique_tuples(int program_point, int **list, int N_tuples, int *tuple_count, Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  void _generate_all_possible_unique_tuples(int program_point, int **list, int N_tuples, int *tuple_count, Ast *node) {
    void _bst_to_list(IntSet *set, int **list, int j, int *i, int repeatition) {
      if (!set) return;
      _bst_to_list(set->left, list, j, i, repeatition);
      int k;
      for (k = 0; k < repeatition; ++k) {
        list[j][*i] = set->value;
        *i += 1;
      }
      _bst_to_list(set->right, list, j, i, repeatition);
    }

    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int j, index = get_symbol_table_index(node->token);
        if (visited[index]) return;
        visited[index] = true;
        for (j = 0; j < N_variables; ++j)
          if (symbol_table_indices[j] == index)
            break;
        if (symbol_table[index].concrete[program_point]) {
          list[j] = (int *) calloc(N_tuples, sizeof(int));
          *tuple_count /= ((ConcreteState *) symbol_table[index].concrete[program_point])->number_of_values;
          int idx = 0, i, k = 0;
          _bst_to_list(((ConcreteState *) symbol_table[index].concrete[program_point])->value_set, list, j, &idx, *tuple_count);
          for (i = idx; i < N_tuples; i++, k = (k+1) % idx)
            list[j][i] = list[j][k];
        }
      }
    }
    int i = node->number_of_children;
    while (i) _generate_all_possible_unique_tuples(program_point, list, N_tuples, tuple_count, node->children[--i]);
  }

  _generate_all_possible_unique_tuples(program_point, list, N_tuples, tuple_count, node);
}

void evaluate(Ast *node, int **list, int N_tuples, ConcreteState *state, int program_point, bool is_logical) {
  int stack[1024], top = 0;
  double probability = 1.0;
  void _postfix_traversal(Ast *node, int i, double *probability, bool visited[]) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int j, index = get_symbol_table_index(node->token);
        if (!visited[index] && !is_logical) *probability *= ((ConcreteState *) symbol_table[index].concrete[program_point])->probability;
        visited[index] = true;
        for (j = 0; j < N_variables; ++j)
          if (symbol_table_indices[j] == index)
            break;
        stack[top++] = list[j][i];
      }
      else
        stack[top++] = atoi(node->token);
      return;
    }
    int j;
    for (j = 0; j < node->number_of_children; ++j)
      _postfix_traversal(node->children[j], i, probability, visited);
    int result = 0, operand1, operand2 = stack[--top];
    if (node->number_of_children == 2)
      operand1 = stack[--top];
    if (!strcmp(node->token, "+")) {
      *probability *= PR_ARITH(PR_ADD);
      result = operand1 + operand2;
    }
    else if (!strcmp(node->token, "-")) {
      *probability *= PR_ARITH(PR_SUB);
      result = operand1 - operand2;
    }
    else if (!strcmp(node->token, "*")) {
      *probability *= PR_ARITH(PR_MUL);
      result = operand1 * operand2;
    }
    else if (!strcmp(node->token, "/")) {
      *probability *= PR_ARITH(PR_DIV);
      result = operand1 / operand2;
    }
    else if (!strcmp(node->token, "%")) {
      *probability *= PR_ARITH(PR_REM);
      result = operand1 % operand2;
    }
    else if (!strcmp(node->token, "==")) {
      *probability *= PR_BOOL(PR_EQ);
      result = operand1 == operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "!=")) {
      *probability *= PR_BOOL(PR_NEQ);
      result = operand1 != operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, ">=")) {
      *probability *= PR_BOOL(PR_GEQ);
      result = operand1 >= operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "<=")) {
      *probability *= PR_BOOL(PR_LEQ);
      result = operand1 <= operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, ">")) {
      *probability *= PR_BOOL(PR_GT);
      result = operand1 > operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "<")) {
      *probability *= PR_BOOL(PR_LT);
      result = operand1 < operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "!")) {
      *probability *= PR_BOOL(PR_NEG);
      result = operand2 ? 0 : 1;
    }
    stack[top++] = result;
  }

  int i;
  list[N_variables] = (int *) calloc(N_tuples, sizeof(int));
  for (i = 0; i < N_tuples; ++i) {
    probability = 1.0;
    bool visited[SYMBOL_TABLE_SIZE] = {false};
    _postfix_traversal(node, i, &probability, visited);
    list[N_variables][i] = limit(stack[--top]);
  }
  state->probability *= probability;
}

void copy_state(ConcreteState *src, ConcreteState *dest) {
  void _traverse(IntSet *set) {
    if (!set) return;
    _traverse(set->left);
    insert_into_concrete_state_value_set(dest, set->value);
    _traverse(set->right);
  }

  if (!src) {
    dest->is_top_element = true;
    return;
  }
  _traverse(src->value_set);
  dest->probability = src->probability;
  dest->is_top_element = src->is_top_element;
}

static void evaluate_expression(Ast *node, int from_program_point) {
  Ast *evaluation_node = node;
  if (node->type == ASSIGNBLOCK)
    evaluation_node = node->children[1];
  free_concrete_state(node->value);
  if (!empty_state_status[from_program_point]) {
    node->value = NULL;
    return;
  }
  node->value = init_concrete_state();
  int **all_possible_tuples = (int **) calloc(N_variables + 1, sizeof(int *)), N_tuples, N_tuples_copy, N_unique_variables, i, j;
  N_tuples = N_tuples_copy = number_of_all_tuples_of_unique_variables(from_program_point, evaluation_node);
  generate_all_possible_unique_tuples(from_program_point, all_possible_tuples, N_tuples, &N_tuples_copy, evaluation_node);
  evaluate(evaluation_node, all_possible_tuples, N_tuples, (ConcreteState *) node->value, from_program_point, node->type == ASSIGNBLOCK ? false : true);
  i = N_unique_variables = get_number_of_unique_variables(evaluation_node);
  while (i--) ((ConcreteState *) node->value)->probability *= PR_ARITH(PR_RD);
  ((ConcreteState *) node->value)->component_states = (ConcreteState **) calloc(N_variables, sizeof(ConcreteState *));

  #ifdef CONCRETE_DEBUG
  printf("\n....................... evaluation steps .......................\n");
  for (i = 0; i < N_variables; ++i) {
    if (all_possible_tuples[i]) {
      printf("%s : [", symbol_table[symbol_table_indices[i]].id);
      int j;
      for (j = 0; j < N_tuples; ++j)
        printf("%d  ", all_possible_tuples[i][j]);
      printf("\b\b]\n");
    }
  }
  if (all_possible_tuples[N_variables]) {
    printf("result : [");
    int j;
    for (j = 0; j < N_tuples; ++j)
      printf("%d  ", all_possible_tuples[N_variables][j]);
    printf("\b\b]\n");
  }
  printf("................................................................\n");
  #endif

  if (node->type == ASSIGNBLOCK) {
    int dest_index = -1, index = get_symbol_table_index(node->children[0]->token);
    while (dest_index < N_variables && symbol_table_indices[++dest_index] != index);
    for (i = 0; i < N_variables; ++i) {
      ((ConcreteState *) node->value)->component_states[i] = init_concrete_state();
      if (i != dest_index)
        copy_state((ConcreteState *) symbol_table[symbol_table_indices[i]].concrete[from_program_point],
            ((ConcreteState *) node->value)->component_states[i]);
      else if (all_possible_tuples[N_variables]) {
        for (j = 0; j < N_tuples; ++j)
          insert_into_concrete_state_value_set(((ConcreteState *) node->value)->component_states[i],
            all_possible_tuples[N_variables][j]);
        ((ConcreteState *) node->value)->component_states[i]->probability = PR_ARITH(PR_WR) * ((ConcreteState *) node->value)->probability;
      }
    }
  }
  else if (node->type == LOGOPBLOCK) {
    bool is_true = false;
    for (i = 0; i < N_variables; ++i) {
      ((ConcreteState *) node->value)->component_states[i] = init_concrete_state();
      if (all_possible_tuples[i]) {
        for (j = 0; j < N_tuples; ++j) {
          if (all_possible_tuples[N_variables][j]) {
            is_true = true;
            insert_into_concrete_state_value_set(((ConcreteState *) node->value)->component_states[i], all_possible_tuples[i][j]);
          }
        }
      }
    }
    if (!N_unique_variables) is_true = (bool) all_possible_tuples[N_variables][0];
    if (!is_true) node->value = NULL;
    else {
      for (i = 0; i < N_variables; ++i) {
        if (!all_possible_tuples[i])
          copy_state((ConcreteState *) symbol_table[symbol_table_indices[i]].concrete[from_program_point],
            ((ConcreteState *) node->value)->component_states[i]);
        ((ConcreteState *) node->value)->component_states[i]->probability =
        ((ConcreteState *) symbol_table[symbol_table_indices[i]].concrete[from_program_point])->probability *
        ((ConcreteState *) node->value)->probability;
      }
    }
  }
  for (i = 0; i <= N_variables; ++i)
    if (all_possible_tuples[i])
      free(all_possible_tuples[i]);
  free(all_possible_tuples);
}

static void upper_bound(ConcreteState **state1, ConcreteState *state2) {
  void _traverse(ConcreteState *state, IntSet *set) {
    if (!set) return;
    _traverse(state, set->left);
    insert_into_concrete_state_value_set(state, set->value);
    _traverse(state, set->right);
  }

  if (!state2) return;
  if (!(*state1)) {
    *state1 = state2;
    return;
  }
  int i;
  for (i = 0; i < N_variables; ++i) {
    _traverse((*state1)->component_states[i], state2->component_states[i]->value_set);
    (*state1)->component_states[i]->probability =
    (*state1)->component_states[i]->probability < state2->component_states[i]->probability ?
    (*state1)->component_states[i]->probability : state2->component_states[i]->probability;
  }
  free_concrete_state(state2);
}

void print_concrete_state(ConcreteState *state) {
  void _inorder_set_traversal(IntSet *set) {
    if (!set) return;
    _inorder_set_traversal(set->left);
    printf("%d ", set->value);
    _inorder_set_traversal(set->right);
  }
  if (!state) return;

  if (state->component_states) {
    int i;
    for (i = 0; i < N_variables; ++i) {
      if (state->component_states[i]) {
        printf("    %s=<{", symbol_table[symbol_table_indices[i]].id);
        _inorder_set_traversal(state->component_states[i]->value_set);
        if (state->component_states[i]->is_top_element)
          printf("a∈ ℤ | m ≤ a ≤ M}, 1>");
        else
          printf("%s, %.12lg>", state->component_states[i]->number_of_values ? "\b}" : "}", state->component_states[i]->probability);
      }
    }
  }
  else {
    _inorder_set_traversal(state->value_set);
    if (state->is_top_element)
      printf("a∈ ℤ | m ≤ a ≤ M}, 1>");
    else
      printf("%s, %.12lg>", state->number_of_values ? "\b}" : "}", state->probability);
  }
}

bool is_equal_sets(ConcreteState *state1, ConcreteState *state2) {
  if (!state1 && !state2) return true;
  if (!state1 || !state2) return false;
  IntSet *set1 = state1->value_set, *set2 = state2->value_set;
  bool is_set1_subset = true, is_set2_subset = true;

  void _is_subset(IntSet *traverse_set, IntSet *match_set, bool *found) {
    bool _does_exists(int key, IntSet *set) {
      if (!set) return false;
      if (set->value == key) return true;
      if (set->value > key) return _does_exists(key, set->left);
      else return _does_exists(key, set->right);
    }
    if (!traverse_set) return;
    *found &= _does_exists(traverse_set->value, match_set);
    _is_subset(traverse_set->left, match_set, found);
    _is_subset(traverse_set->right, match_set, found);
  }
  _is_subset(set1, set2, &is_set1_subset);
  _is_subset(set2, set1, &is_set2_subset);
  return is_set1_subset & is_set2_subset;
}

static bool iterate(int initial_program_point) {

  bool has_fixed_point_reached = true;
  void _iterate(bool iterate_exit_program_point) {
    int i, j, k, start = iterate_exit_program_point ? 0 : initial_program_point + 1, end = iterate_exit_program_point ? 0 : N_lines;
    for (i = start; i <= end; i++) {
      bool is_first_eqn = true, program_point_is_fixed = true;
      ConcreteState *previous_state = NULL;

      #ifdef CONCRETE_DEBUG
      printf("\n==============================================================================\n");
      #endif

      for (j = 0; j <= N_lines; j++) {
        if (data_flow_graph[i][j]) {

          #ifdef CONCRETE_DEBUG
          char stmt[200] = {0};
          printf("G_%d <- G_%d [ %s ] %s\n", i ? i : N_lines + 1, j ? j : N_lines + 1, expression_to_string(data_flow_matrix[i][j], stmt),
            data_flow_matrix[i][j]->type == LOGOPBLOCK ? "logical" : (data_flow_matrix[i][j]->type == ASSIGNBLOCK ? "assignment" : ""));
          #endif

          if (is_first_eqn) {
            is_first_eqn = false;
            evaluate_expression(data_flow_matrix[i][j], j);

            #ifdef CONCRETE_DEBUG
            printf("evaluated result :: \t");
            print_concrete_state((ConcreteState *) data_flow_matrix[i][j]->value);
            printf("\n");
            #endif

            previous_state = (ConcreteState *) data_flow_matrix[i][j]->value;
            data_flow_matrix[i][j]->value = NULL;
            continue;
          }
          evaluate_expression(data_flow_matrix[i][j], j);

          #ifdef CONCRETE_DEBUG
          printf("evaluated result ::\t");
          print_concrete_state((ConcreteState *) data_flow_matrix[i][j]->value);
          #endif

          upper_bound(&previous_state, (ConcreteState *) data_flow_matrix[i][j]->value);
          data_flow_matrix[i][j]->value = NULL;

          #ifdef CONCRETE_DEBUG
          printf("\nupper bound ::\t");
          print_concrete_state(previous_state);
          printf("\n");
          #endif
        }
      }
      if (!is_first_eqn) {
        empty_state_status[i] = previous_state ? true : false;
        for (k = 0; k < N_variables; ++k) {
          bool is_fixed = is_equal_sets(symbol_table[symbol_table_indices[k]].concrete[i], previous_state ? previous_state->component_states[k] : NULL);
          program_point_is_fixed &= is_fixed;
          
          #ifdef CONCRETE_DEBUG
          printf("%s is %s\t", symbol_table[symbol_table_indices[k]].id, is_fixed ? "fixed" : "NOT fixed");
          #endif

          if (!is_fixed) {
            free_concrete_state(symbol_table[symbol_table_indices[k]].concrete[i]);
            symbol_table[symbol_table_indices[k]].concrete[i] = previous_state ? previous_state->component_states[k] : NULL;
          }
          else if (previous_state)
            free_concrete_state(previous_state->component_states[k]);
        }

        #ifdef CONCRETE_DEBUG
        printf("\nG_%d (%s) ::\t", i ? i : N_lines + 1, program_point_is_fixed ? "fixed" : "NOT fixed");
        for (k = 0; k < N_variables; ++k) {
          printf("  %s∈ <{", symbol_table[symbol_table_indices[k]].id);
          print_concrete_state(symbol_table[symbol_table_indices[k]].concrete[i]);
          printf("%s", symbol_table[symbol_table_indices[k]].concrete[i] ? "" : "}, 1.0>");
        }
        printf("\n==============================================================================\n");
        #endif
      }
      has_fixed_point_reached &= program_point_is_fixed;
    }
  }

  _iterate(false);
  _iterate(true);
  return has_fixed_point_reached;
}

void concrete_analysis() {
  /*
    if (MAXINT ~ MININT) ≈ 20 then the parameter passed to initialize_first_program_point
    MUST be false in order to avoid segmentation fault. And in that case EVERY VARIABLE MUST
    BE INITIALIZED in the begining.
  */
  int i = 0, initial_program_point = initialize_first_program_point(false);
  bool is_fixed;
  
  do {
    i += 1;
    
    #ifdef CONCRETE_DEBUG
    printf("\t\t\t\titeration-%d", i);
    #endif

    is_fixed = iterate(initial_program_point);
  } while(!is_fixed && i < MAX_ITERATION);

  is_solution_fixed = is_fixed;
  N_iterations = i;
  first_program_point = initial_program_point;

  free(empty_state_status);
  for (i = 0; i <= N_lines; i++)
    free(data_flow_graph[i]);
  free(data_flow_graph);
}

void print_concrete_analysis_result() {
  
  int i;
  if (is_solution_fixed) printf("Reached fixed point after %d iterations\n\n", N_iterations);
  else printf("Fixed point is not reached within %d iterations\n\n", N_iterations);

  for (i = 1; i <= N_lines; i++) {
    if (data_flow_matrix[i]) {
      printf("G_%d  ::  ", i);
      int j;
      for (j = 0; j < N_variables; ++j) {
        printf("    %s=<{", symbol_table[symbol_table_indices[j]].id);
        if (i == first_program_point)
          printf("a∈ ℤ | m ≤ a ≤ M}, 1>");
        else if (symbol_table[symbol_table_indices[j]].concrete[i])
          print_concrete_state(symbol_table[symbol_table_indices[j]].concrete[i]);
        else
          printf("}, 1>");
      }
      printf("\n");
    }
  }
  if (data_flow_matrix[0]) {
    printf("G_%d  ::  ", N_lines + 1);
    int j;
    for (j = 0; j < N_variables; ++j) {
      printf("    %s=<{", symbol_table[symbol_table_indices[j]].id);
      if (symbol_table[symbol_table_indices[j]].concrete[0])
        print_concrete_state(symbol_table[symbol_table_indices[j]].concrete[0]);
      else
        printf("}, 1>");
    }
    printf("\n");
  }
}