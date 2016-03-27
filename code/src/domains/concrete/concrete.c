#include "concrete.h"

size_t N_concrete_iterations, first_concrete_program_point;
bool is_concrete_solution_fixed;

ConcreteState *init_concrete_state() {
  ConcreteState *state = (ConcreteState *) calloc(1, sizeof(ConcreteState));
  state->probability = 1.0;
  return state;
}

void free_concrete_state(ConcreteState *state) {
  void _free(IntegerSet *set) {
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

void insert_into_concrete_state_value_set(ConcreteState *state, var_t value) {
  if (state) {
    bool key_exists = false;
    state->value_set = avl_tree_insert(state->value_set, value, &key_exists);
    state->number_of_values += key_exists ? 0 : 1;
  }
}

static size_t initialize_first_program_point(bool initialize_all_uninitialized_variables_with_all_values_from_the_range) {
  size_t i, j;
  empty_state_status = (bool *) calloc(N_stmts + 1, sizeof(bool));
  data_flow_graph = (bool **) calloc(N_stmts + 1, sizeof(bool *));  
  for (i = 0; i <= N_stmts; i++) {
    data_flow_graph[i] = (bool *) calloc(N_stmts + 1, sizeof(bool));
    for (j = 0; j <= N_stmts; j++)
      data_flow_graph[i][j] = data_flow_matrix[i] ? (data_flow_matrix[i][j] ? true : false) : false;
  }
  for (i = 1; i <= N_stmts; i++) {
    if (data_flow_matrix[i]) {
      empty_state_status[i] = true;
      for (j = 0; j < N_variables; ++j) {
        if (!symbol_table[symbol_table_indices[j]].concrete)
          symbol_table[symbol_table_indices[j]].concrete = calloc(N_stmts + 1, sizeof(ConcreteState *));
        if (initialize_all_uninitialized_variables_with_all_values_from_the_range) {
          symbol_table[symbol_table_indices[j]].concrete[i] = calloc(1, sizeof(ConcreteState));
          ((ConcreteState *) symbol_table[symbol_table_indices[j]].concrete[i])->probability = 1.0;
          var_t k;
          for (k = MININT; k <= MAXINT; ++k)
            insert_into_concrete_state_value_set((ConcreteState *) symbol_table[symbol_table_indices[j]].concrete[i], k);
        }
      }
      break;
    }
  }
  return i;
}

void copy_state(ConcreteState *src, ConcreteState *dest) {
  void _traverse(IntegerSet *set) {
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

void evaluate(bool verbose, Ast *node, ConcreteState *state, ConcreteAllTuple *list, size_t N_list, size_t program_point, bool is_logical,
              int dest_index, bool *is_true) {
  var_t stack[EVALUATION_STACK_SIZE];
  size_t top = 0;
  long double probability = 1.0;
  void _postfix_traversal(Ast *node, long double *probability, bool visited[]) {
  	size_t j;
    if (!node->number_of_children) {
      if (node->type & IDBLOCK) {
        size_t index = symbol_table_entry(node->token);
        if (!visited[index] && !is_logical) *probability *= ((ConcreteState *) symbol_table[index].concrete[program_point])->probability;
        visited[index] = true;
        for (j = 0; j < N_list; ++j)
          if (list[j].symbol_table_index == index)
            break;
        stack[top++] = list[j].values[list[j].current_index];
      }
      else
        stack[top++] = (var_t) atol(node->token);
      return;
    }
    for (j = 0; j < node->number_of_children; ++j)
      _postfix_traversal(node->children[j], probability, visited);
    var_t result = 0, operand1, operand2 = stack[--top];
    if (node->number_of_children == 2)
      operand1 = stack[--top];
    switch (node->operator_type) {
      case ADD:
        *probability *= PR_ARITH(PR_ADD);
        result = operand1 + operand2;
        break;
      case SUB:
        *probability *= PR_ARITH(PR_SUB);
        result = operand1 - operand2;
        break;
      case MUL:
        *probability *= PR_ARITH(PR_MUL);
        result = operand1 * operand2;
        break;
      case DIV:
        *probability *= PR_ARITH(PR_DIV);
        result = operand1 / operand2;
        break;
      case REM:
        *probability *= PR_ARITH(PR_REM);
        result = operand1 % operand2;
        break;
      case EQ:
        *probability *= PR_BOOL(PR_EQ);
        result = operand1 == operand2 ? 1 : 0;
        break;
      case NEQ:
        *probability *= PR_BOOL(PR_NEQ);
        result = operand1 != operand2 ? 1 : 0;
        break;
      case GEQ:
        *probability *= PR_BOOL(PR_GEQ);
        result = operand1 >= operand2 ? 1 : 0;
        break;
      case LEQ:
        *probability *= PR_BOOL(PR_LEQ);
        result = operand1 <= operand2 ? 1 : 0;
        break;
      case GT:
        *probability *= PR_BOOL(PR_GT);
        result = operand1 > operand2 ? 1 : 0;
        break;
      case LT:
        *probability *= PR_BOOL(PR_LT);
        result = operand1 < operand2 ? 1 : 0;
        break;
      case NOT:
        *probability *= PR_BOOL(PR_NEG);
        result = operand2 ? 0 : 1;
        break;
      case LAND:
        *probability *= PR_BOOL(PR_AND);
        result = (operand1 && operand2) ? 1 : 0;
        break;
      case LOR:
        *probability *= PR_BOOL(PR_OR);
        result = (operand1 || operand2) ? 1 : 0;
    }
    stack[top++] = result;
  }

  int j;

  if (verbose) {
    printf("\n.............................. evaluation steps ..............................\n");
    for (j = 0; j < N_list; ++j)
  	 printf("%s%s, ", j == 0 ? "(" : "", symbol_table[list[j].symbol_table_index].id);
    printf("Result%s  ::  ", j ? ")" : "");
  }
  	
  while (true) {
    probability = 1.0;
    bool visited[SYMBOL_TABLE_SIZE] = {false};
    _postfix_traversal(node, &probability, visited);

    if (verbose) {
      for (j = 0; j < N_list; ++j)
        printf("%s%ld,", j == 0 ? "(" : "", (long) list[j].values[list[j].current_index]);
      printf("%ld%s  ", (long) limit(stack[top-1]), j ? ")" : "");
    }

    if (!is_logical)
      insert_into_concrete_state_value_set(state->component_states[dest_index], limit(stack[--top]));
  	else {
  	  if (stack[--top]) {
  	  	*is_true = true;
  	  	for (j = 0; j < N_list; ++j)
  	  	  insert_into_concrete_state_value_set(state->component_states[list[j].symbol_table_indices_list_index], list[j].values[list[j].current_index]);
  	  }
  	}

    if (!N_list)
      break;
    j = N_list - 1;
    list[j].current_index += 1;
    while (j > 0 && list[j].current_index >= list[j].max_values) {
      list[j].current_index = 0;
      j--;
      list[j].current_index += 1;
    }
    if (j == 0 && list[j].current_index >= list[j].max_values)
      break;
  }

  state->probability *= probability;

  if (verbose) printf("\n..............................................................................\n");
}

static void populate_tuples_list(Ast *node, ConcreteAllTuple *map, size_t program_point) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  size_t index = 0, i;

  void _bst_to_list(IntegerSet *set, var_t *list) {
  	if (!set) return;
  	_bst_to_list(set->left, list);
  	list[i++] = set->value;
    _bst_to_list(set->right, list);
  }
  void _traverse(Ast *node) {
    if (!node->number_of_children) {
      if (node->type & IDBLOCK) {
        size_t idx = symbol_table_entry(node->token);
        if (visited[idx]) return;
        visited[idx] = true;
        map[index].symbol_table_index = idx;
        for (i = 0; i < N_variables; ++i) {
      	  if (symbol_table_indices[i] == idx) {
      	  	map[index].symbol_table_indices_list_index = i;
      	  	break;
      	  }
      	}
        ConcreteState *state = (ConcreteState *) symbol_table[idx].concrete[program_point];
        map[index].max_values = state ? state->number_of_values : 0;
        map[index].values = (var_t *) calloc(map[index].max_values, sizeof(var_t));
        i = 0;
        _bst_to_list(state ? state->value_set : NULL, map[index].values);
        index += 1;
      }
      return;
    }
    int i = node->number_of_children;
    while (i) _traverse(node->children[--i]);
  }
  
  _traverse(node);
}

static void evaluate_expression(bool verbose, Ast *node, size_t from_program_point) {
  Ast *evaluation_node = node;
  if (node->type & ASSIGNBLOCK)
    evaluation_node = node->children[1];
  free_concrete_state(node->value);
  if (!empty_state_status[from_program_point]) {
    node->value = NULL;
    return;
  }
  node->value = init_concrete_state();
  size_t N_unique_variables = get_number_of_unique_variables(evaluation_node);
  ConcreteAllTuple *tuples = (ConcreteAllTuple *) calloc(N_unique_variables, sizeof(ConcreteAllTuple));
  populate_tuples_list(evaluation_node, tuples, from_program_point);

  int i = N_unique_variables;
  while (i--) ((ConcreteState *) node->value)->probability *= PR_ARITH(PR_RD);
  ((ConcreteState *) node->value)->component_states = (ConcreteState **) calloc(N_variables, sizeof(ConcreteState *));
  for (i = 0; i < N_variables; ++i)
  	((ConcreteState *) node->value)->component_states[i] = init_concrete_state();

  int dest_index = -1;
  bool is_true = false;
  if (node->type & ASSIGNBLOCK) {
    size_t index = symbol_table_entry(node->children[0]->token);
    while (dest_index < N_variables && symbol_table_indices[++dest_index] != index);
    for (i = 0; i < N_variables; ++i)
      if (i != dest_index)
        copy_state((ConcreteState *) symbol_table[symbol_table_indices[i]].concrete[from_program_point],
        	((ConcreteState *) node->value)->component_states[i]);
  }

  evaluate(verbose, evaluation_node, (ConcreteState *) node->value, tuples, N_unique_variables, from_program_point,
    node->type & ASSIGNBLOCK ? false : true, dest_index, &is_true);

  if (node->type & ASSIGNBLOCK)
  	((ConcreteState *) node->value)->component_states[dest_index]->probability = PR_ARITH(PR_WR) * ((ConcreteState *) node->value)->probability;
  else {
    if (!is_true)  {
      for (i = 0; i < N_variables; ++i)
      	free(((ConcreteState *) node->value)->component_states[i]);
      free(((ConcreteState *) node->value)->component_states);
      node->value = NULL;
    }
    else {
      bool *no_need_to_be_copied = (bool *) calloc(N_variables, sizeof(bool));
      for (i = 0; i < N_unique_variables; ++i)
      	no_need_to_be_copied[tuples[i].symbol_table_indices_list_index] = true;
      for (i = 0; i < N_variables; ++i) {
        if (!no_need_to_be_copied[i])
          copy_state((ConcreteState *) symbol_table[symbol_table_indices[i]].concrete[from_program_point],
            ((ConcreteState *) node->value)->component_states[i]);
        ((ConcreteState *) node->value)->component_states[i]->probability =
        ((ConcreteState *) symbol_table[symbol_table_indices[i]].concrete[from_program_point])->probability *
        ((ConcreteState *) node->value)->probability;
      }
      free(no_need_to_be_copied);
    }
  }
  for (i = 0; i < N_unique_variables; ++i)
  	free(tuples[i].values);
  free(tuples);
}

static void upper_bound(ConcreteState **state1, ConcreteState *state2) {
  void _traverse(ConcreteState *state, IntegerSet *set) {
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
  size_t i;
  for (i = 0; i < N_variables; ++i) {
    _traverse((*state1)->component_states[i], state2->component_states[i]->value_set);
    (*state1)->component_states[i]->probability =
    (*state1)->component_states[i]->probability < state2->component_states[i]->probability ?
    (*state1)->component_states[i]->probability : state2->component_states[i]->probability;
  }
  free_concrete_state(state2);
}

void print_concrete_state(FILE *stream, ConcreteState *state) {
  void _inorder_set_traversal(IntegerSet *set) {
    if (!set) return;
    _inorder_set_traversal(set->left);
    fprintf(stream, "%ld ", (long) set->value);
    _inorder_set_traversal(set->right);
  }
  if (!state) return;

  if (state->component_states) {
    size_t i;
    for (i = 0; i < N_variables; ++i) {
      if (state->component_states[i]) {
        fprintf(stream, "\t%s=<{", symbol_table[symbol_table_indices[i]].id);
        _inorder_set_traversal(state->component_states[i]->value_set);
        if (state->component_states[i]->is_top_element)
          fprintf(stream, "a∈ ℤ | m ≤ a ≤ M}, 1>");
        else
          fprintf(stream, "%s, %.12Lg>", state->component_states[i]->number_of_values ? "\b}" : "}", state->component_states[i]->probability);
      }
    }
  }
  else {
    _inorder_set_traversal(state->value_set);
    if (state->is_top_element)
      fprintf(stream, "a∈ ℤ | m ≤ a ≤ M}, 1>");
    else
      fprintf(stream, "%s, %.12Lg>", state->number_of_values ? "\b}" : "}", state->probability);
  }
}

bool is_equal_sets(ConcreteState *state1, ConcreteState *state2) {
  if (!state1 && !state2) return true;
  if (!state1 || !state2) return false;
  IntegerSet *set1 = state1->value_set, *set2 = state2->value_set;
  bool is_set1_subset = true, is_set2_subset = true;

  void _is_subset(IntegerSet *traverse_set, IntegerSet *match_set, bool *found) {
    bool _does_exists(var_t key, IntegerSet *set) {
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

static bool iterate(bool verbose, size_t initial_program_point) {

  bool has_fixed_point_reached = true;
  void _iterate(bool iterate_exit_program_point) {
    size_t i, j, k, start = iterate_exit_program_point ? 0 : initial_program_point + 1, end = iterate_exit_program_point ? 0 : N_stmts;
    for (i = start; i <= end; i++) {
      bool is_first_eqn = true, program_point_is_fixed = true;
      ConcreteState *previous_state = NULL;

      if (verbose) printf("\n==============================================================================\n");

      for (j = 0; j <= N_stmts; j++) {
        if (data_flow_graph[i][j]) {

          if (verbose) {
            char stmt[200] = {0}, subscript[200];
            printf("G%s <- ", print_subsscript(i ? i : N_stmts + 1, subscript));
            printf("G%s [ %s ] %s\n", print_subsscript(j ? j : N_stmts + 1, subscript), expression_to_string(data_flow_matrix[i][j], stmt),
              data_flow_matrix[i][j]->type & LOGOPBLOCK ? "logical" : (data_flow_matrix[i][j]->type & ASSIGNBLOCK ? "assignment" : ""));
          }

          if (is_first_eqn) {
            is_first_eqn = false;
            evaluate_expression(verbose, data_flow_matrix[i][j], j);

            if (verbose) {
              printf("evaluated result ::\n\t");
              print_concrete_state(stdout, (ConcreteState *) data_flow_matrix[i][j]->value);
              printf("\n\n");
            }

            previous_state = (ConcreteState *) data_flow_matrix[i][j]->value;
            data_flow_matrix[i][j]->value = NULL;
            continue;
          }
          evaluate_expression(verbose, data_flow_matrix[i][j], j);

          if (verbose) {
            printf("evaluated result ::\n\t");
            print_concrete_state(stdout, (ConcreteState *) data_flow_matrix[i][j]->value);
          }

          upper_bound(&previous_state, (ConcreteState *) data_flow_matrix[i][j]->value);
          data_flow_matrix[i][j]->value = NULL;

          if (verbose) {
            printf("\n\nupper bound ::\n\t");
            print_concrete_state(stdout, previous_state);
            printf("\n\n");
          }
        }
      }
      if (!is_first_eqn) {
        empty_state_status[i] = previous_state ? true : false;
        for (k = 0; k < N_variables; ++k) {
          bool is_fixed = is_equal_sets(symbol_table[symbol_table_indices[k]].concrete[i], previous_state ? previous_state->component_states[k] : NULL);
          program_point_is_fixed &= is_fixed;
          
          if (verbose) printf("%s is %s\t\t", symbol_table[symbol_table_indices[k]].id, is_fixed ? "fixed" : "NOT fixed");

          if (!is_fixed) {
            free_concrete_state(symbol_table[symbol_table_indices[k]].concrete[i]);
            symbol_table[symbol_table_indices[k]].concrete[i] = previous_state ? previous_state->component_states[k] : NULL;
          }
          else if (previous_state)
            free_concrete_state(previous_state->component_states[k]);
        }

        if (verbose) {
          char subscript[200];
          printf("\n\nG%s (%s) ::\n", print_subsscript(i ? i : N_stmts + 1, subscript), program_point_is_fixed ? "fixed" : "NOT fixed");
          for (k = 0; k < N_variables; ++k) {
            printf("\t%s∈ <{", symbol_table[symbol_table_indices[k]].id);
            print_concrete_state(stdout, symbol_table[symbol_table_indices[k]].concrete[i]);
            printf("%s", symbol_table[symbol_table_indices[k]].concrete[i] ? "" : "}, 1.0>");
          }
          printf("\n==============================================================================\n");
        }
      }
      has_fixed_point_reached &= program_point_is_fixed;
    }
  }

  _iterate(false);
  _iterate(true);
  return has_fixed_point_reached;
}

void concrete_analysis(bool verbose, size_t iteration) {
  /*
    if (MAXINT ~ MININT) ≈ 20 then the parameter passed to initialize_first_program_point
    MUST be false in order to avoid segmentation fault. And in that case EVERY VARIABLE MUST
    BE INITIALIZED in the begining.
  */
  size_t i = 0, initial_program_point = initialize_first_program_point(false);
  bool is_fixed;
  
  do {
    i += 1;
    
    if (verbose) printf("\t\t\t\titeration-%zu", i);

    is_fixed = iterate(verbose, initial_program_point);
  } while(!is_fixed && i < iteration);

  is_concrete_solution_fixed = is_fixed;
  N_concrete_iterations = i;
  first_concrete_program_point = initial_program_point;

  free(empty_state_status);
  for (i = 0; i <= N_stmts; i++)
    free(data_flow_graph[i]);
  free(data_flow_graph);
}

void print_concrete_analysis_result(FILE *stream, size_t N_columns) {
  size_t i, last_line_no;
  char subscript[200];
  if (is_concrete_solution_fixed) fprintf(stream, "\nReached fixed point after %zu iterations\n\n", N_concrete_iterations);
  else fprintf(stream, "\nFixed point is NOT reached within %zu iterations\n\n", N_concrete_iterations);

  for (i = 1; i <= N_stmts; i++) {
    if (data_flow_matrix[i]) {
      last_line_no = stmt_line_map[i];
      fprintf(stream, "G%s (line #%zu) :", print_subsscript(i, subscript), last_line_no);
      int j;
      for (j = 0; j < N_variables; ++j) {
        fprintf(stream, "%s\t\t%s=<{", j % N_columns ? "" : "\n", symbol_table[symbol_table_indices[j]].id);
        if (i == first_concrete_program_point)
          fprintf(stream, "a∈ ℤ | m ≤ a ≤ M}, 1>");
        else if (symbol_table[symbol_table_indices[j]].concrete[i])
          print_concrete_state(stream, symbol_table[symbol_table_indices[j]].concrete[i]);
        else
          fprintf(stream, "}, 1>");
      }
      fprintf(stream, "\n");
    }
  }
  if (data_flow_matrix[0]) {
    fprintf(stream, "G%s (line #%zu) :", print_subsscript(N_stmts + 1, subscript), ++last_line_no);
    int j;
    for (j = 0; j < N_variables; ++j) {
      fprintf(stream, "%s\t\t%s=<{", j % N_columns ? "" : "\n", symbol_table[symbol_table_indices[j]].id);
      if (symbol_table[symbol_table_indices[j]].concrete[0])
        print_concrete_state(stream, symbol_table[symbol_table_indices[j]].concrete[0]);
      else
        fprintf(stream, "}, 1>");
    }
    fprintf(stream, "\n");
  }
}