#include "concrete.h"
#include "dataflow/parser.h"

#define is_id_block(n) ((n->type == IDBLOCK || (n->type == LOGOPBLOCK && !atoi(n->token) && n->token[0] != '0')) ? true : false)


ConcreteState *init_concrete_state() {
  ConcreteState *state = (ConcreteState *) calloc(1, sizeof(ConcreteState));
  state->probability = 1.0;
  return state;
}

void free_concrete_state(ConcreteState *state) {
  void _free(ConcreteValueSet *set) {
    if (!set) return;
    _free(set->left);
    _free(set->right);
    free(set);
  }

  if (state) {
    _free(state->value_set);
    if (state->component_states) {
      int i;
      for (i = 0; i <= N_variables; ++i)
        if (state->component_states[i])
          free(state->component_states[i]);
    }
    free(state);
  }
}

void insert_into_concrete_state_value_set(ConcreteState *state, int value) {
  ConcreteValueSet *_avl_tree_insert(ConcreteValueSet *head, int value, bool *is_key_found) {
    ConcreteValueSet *_rotate_right(ConcreteValueSet *y) {
      ConcreteValueSet *x = y->left, *T2 = x->right;
      x->right = y;
      y->left = T2;
      int y_left_height = y->left ? y->left->_height : 0, y_right_height = y->right ? y->right->_height : 0,
          x_left_height = x->left ? x->left->_height : 0, x_right_height = x->right ? x->right->_height : 0;
      y->_height = (y_left_height > y_right_height ? y_left_height : y_right_height) + 1;
      x->_height = (x_left_height > x_right_height ? x_left_height : x_right_height) + 1;
      return x;
    }
    ConcreteValueSet *_rotate_left(ConcreteValueSet *x) {
      ConcreteValueSet *y = x->right, *T2 = y->left;
      y->left = x;
      x->right = T2;
      int y_left_height = y->left ? y->left->_height : 0, y_right_height = y->right ? y->right->_height : 0,
          x_left_height = x->left ? x->left->_height : 0, x_right_height = x->right ? x->right->_height : 0;
      y->_height = (y_left_height > y_right_height ? y_left_height : y_right_height) + 1;
      x->_height = (x_left_height > x_right_height ? x_left_height : x_right_height) + 1;
      return y;
    }
    if (!head) {
      ConcreteValueSet *new_node = (ConcreteValueSet *) calloc(1, sizeof(ConcreteValueSet));
      new_node->value = value;
      new_node->_height = 1;
      return new_node;
    }
    if (value == head->value) {
      *is_key_found = true;
      return head;
    }
    else if (value < head->value) head->left  = _avl_tree_insert(head->left, value, is_key_found);
    else head->right  = _avl_tree_insert(head->right, value, is_key_found);
    if (*is_key_found) return head;
    int head_left_height = head->left ? head->left->_height : 0,
        head_right_height = head->right ? head->right->_height : 0,
        balance = head ? (head_left_height - head_right_height) : 0;
    head->_height = (head_left_height > head_right_height ? head_left_height : head_right_height) + 1;
    
    if (balance > 1 && head->left->value > value) return _rotate_right(head);
    if (balance < -1 && head->right->value < value) return _rotate_left(head);
    if (balance > 1 && head->left->value < value) {
      head->left = _rotate_left(head->left);
      return _rotate_right(head);
    }
    if (balance < -1 && head->right->value > value) {
      head->right = _rotate_right(head->right);
      return _rotate_left(head);
    }
    return head;
  }

  if (state) {
    bool key_exists = false;
    state->value_set = _avl_tree_insert(state->value_set, value, &key_exists);
    state->number_of_values += key_exists ? 0 : 1;
  }
}

void initialize_first_program_point() {
  int i, j, k;
  for (i = 1; i <= N_lines; i++) {
    if (data_flow_matrix[i]) {
      for (j = 0; j < N_variables; ++j) {
        if (!symbol_table[symbol_table_indices[j]].concrete)
          symbol_table[symbol_table_indices[j]].concrete = calloc(N_lines + 1, sizeof(ConcreteState *));
        symbol_table[symbol_table_indices[j]].concrete[i] = calloc(1, sizeof(ConcreteState));
        ((ConcreteState *) symbol_table[symbol_table_indices[j]].concrete[i])->probability = 1.0;
        for (k = MININT; k <= MAXINT; ++k)
          insert_into_concrete_state_value_set((ConcreteState *) symbol_table[symbol_table_indices[j]].concrete[i], k);
      }
      break;
    }
  }
}

bool is_empty_program_point(int program_point, Ast *node) {
  if (!node->number_of_children) {
    if (is_id_block(node)) {
      int index = get_symbol_table_index(node->token);
      if (!symbol_table[index].concrete) symbol_table[index].concrete = calloc(N_lines + 1, sizeof(ConcreteState *));
      return symbol_table[index].concrete[program_point] ?
             (((ConcreteState *) symbol_table[index].concrete[program_point])->number_of_values ? false : true) :
             true;
    }
    return false;
  }
  int i = node->number_of_children;
  bool ret = false;
  while (i) ret |= is_empty_program_point(program_point, node->children[--i]);
  return ret;
}

unsigned int get_number_of_unique_variables(Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  int _get_number_of_unique_variables(Ast *node) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int index = get_symbol_table_index(node->token);
        if (visited[index]) return 0;
        visited[index] = true;
        return 1;
      }
      return 0;
    }
    int i = node->number_of_children, count = 0;
    while (i) count += _get_number_of_unique_variables(node->children[--i]);
    return count;
  }
  
  return _get_number_of_unique_variables(node);
}

int number_of_all_tuples_of_unique_variables(int program_point, Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  int _number_of_all_tuples_of_unique_variables(int program_point, Ast *node) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int index = get_symbol_table_index(node->token);
        if (visited[index]) return 1;
        visited[index] = true;
        if (!symbol_table[index].concrete) symbol_table[index].concrete = calloc(N_lines + 1, sizeof(ConcreteState *));
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

void generate_all_possible_unique_tuples(int program_point, int **list, int N_tuples, int *tuple_count, Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  void _generate_all_possible_unique_tuples(int program_point, int **list, int N_tuples, int *tuple_count, Ast *node) {
    void _bst_to_list(ConcreteValueSet *set, int **list, int j, int *i, int repeatition) {
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
        if (!symbol_table[index].concrete) symbol_table[index].concrete = calloc(N_lines + 1, sizeof(ConcreteState *));
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

void evaluate(Ast *node, int **list, int N_tuples, ConcreteState *state) {
  int stack[1024], top = 0;
  double probability = 1.0;
  void _postfix_traversal(Ast *node, int i, double *probability) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int j, index = get_symbol_table_index(node->token);
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
      _postfix_traversal(node->children[j], i, probability);
    int result = 0, operand1, operand2 = stack[--top];
    if (node->number_of_children == 2)
      operand1 = stack[--top];
    if (!strcmp(node->token, "+")) {
      *probability *= PR_ADD;
      result = operand1 + operand2;
    }
    else if (!strcmp(node->token, "-")) {
      *probability *= PR_SUB;
      result = operand1 - operand2;
    }
    else if (!strcmp(node->token, "*")) {
      *probability *= PR_MUL;
      result = operand1 * operand2;
    }
    else if (!strcmp(node->token, "/")) {
      *probability *= PR_DIV;
      result = operand1 / operand2;
    }
    else if (!strcmp(node->token, "%")) {
      *probability *= PR_REM;
      result = operand1 % operand2;
    }
    else if (!strcmp(node->token, "==")) {
      *probability *= PR_EQ;
      result = operand1 == operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "!=")) {
      *probability *= PR_NEQ;
      result = operand1 != operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, ">=")) {
      *probability *= PR_GEQ;
      result = operand1 >= operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "<=")) {
      *probability *= PR_LEQ;
      result = operand1 <= operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, ">")) {
      *probability *= PR_GT;
      result = operand1 > operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "<")) {
      *probability *= PR_LT;
      result = operand1 < operand2 ? 1 : 0;
    }
    else if (!strcmp(node->token, "!")) {
      *probability *= PR_NEG;
      result = operand2 ? 0 : 1;
    }
    stack[top++] = result;
  }

  int i;
  list[N_variables] = (int *) calloc(N_tuples, sizeof(int));
  for (i = 0; i < N_tuples; ++i) {
    probability = 1.0;
    _postfix_traversal(node, i, &probability);
    list[N_variables][i] = stack[--top];
  }
  state->probability *= probability;
}


void evaluate_expression(Ast *node, int from_program_point) {
  Ast *evaluation_node = node;
  if (node->type == ASSIGNBLOCK)
    evaluation_node = node->children[1];
  free_concrete_state(node->value);
  if (is_empty_program_point(from_program_point, evaluation_node)) {
    node->value = NULL;
    return;
  }
  node->value = init_concrete_state();
  int **all_possible_tuples = (int **) calloc(N_variables + 1, sizeof(int *)), N_tuples, N_tuples_copy;
  N_tuples = N_tuples_copy = number_of_all_tuples_of_unique_variables(from_program_point, evaluation_node);
  generate_all_possible_unique_tuples(from_program_point, all_possible_tuples, N_tuples, &N_tuples_copy, evaluation_node);
  evaluate(evaluation_node, all_possible_tuples, N_tuples, (ConcreteState *) node->value);
  int i = get_number_of_unique_variables(evaluation_node);
  while (i--) ((ConcreteState *) node->value)->probability *= PR_RD;




  // printf("............... printing variable lists ............................\n");
  // for (i = 0; i < N_variables; ++i) {
  //   if (all_possible_tuples[i]) {
  //     printf("%s : [", symbol_table[symbol_table_indices[i]].id);
  //     int j;
  //     for (j = 0; j < N_tuples; ++j)
  //       printf("%d  ", all_possible_tuples[i][j]);
  //     printf("\b\b]\n");
  //   }
  // }
  // if (all_possible_tuples[N_variables]) {
  //   printf("result : [");
  //   int j;
  //   for (j = 0; j < N_tuples; ++j)
  //     printf("%d  ", all_possible_tuples[N_variables][j]);
  //   printf("\b\b]\n");
  // }
  // printf("....................................................................\n");



  
  if (node->type == ASSIGNBLOCK) {
    if (all_possible_tuples[N_variables]) {
      for (i = 0; i < N_tuples; ++i) {
        insert_into_concrete_state_value_set((ConcreteState *) node->value, all_possible_tuples[N_variables][i]);
      }
    }
  }
  else if (node->type == LOGOPBLOCK) {
    bool is_true = false;
    for (i = 0; i < N_variables; ++i) {
      if (all_possible_tuples[i]) {
        if (!((ConcreteState *) node->value)->component_states)
          ((ConcreteState *) node->value)->component_states = (ConcreteState **) calloc(N_variables, sizeof(ConcreteState *));
        ((ConcreteState *) node->value)->component_states[i] = init_concrete_state();
        int j;
        for (j = 0; j < N_tuples; ++j) {
          if (all_possible_tuples[N_variables][j]) {
            is_true = true;
            insert_into_concrete_state_value_set(((ConcreteState *) node->value)->component_states[i], all_possible_tuples[i][j]);
          }
        }
      }
    }
    insert_into_concrete_state_value_set((ConcreteState *) node->value,
      ((ConcreteState *) node->value)->component_states ? is_true : (bool) all_possible_tuples[N_variables][0]);
  }
  for (i = 0; i <= N_variables; ++i)
    if (all_possible_tuples[i])
      free(all_possible_tuples[i]);
  free(all_possible_tuples);
}

void print_concrete_state(ConcreteState *state) {
  void _inorder_set_traversal(ConcreteValueSet *set) {
    if (!set) return;
    _inorder_set_traversal(set->left);
    printf("%d ", set->value);
    _inorder_set_traversal(set->right);
  }
  if (!state) return;
  printf("< {");
  _inorder_set_traversal(state->value_set);
  printf("\b}, %.8lf >", state->probability);
  if (state->component_states) {
    int i;
    printf("    [");
    for (i = 0; i < N_variables; ++i) {
      if (state->component_states[i]) {
        printf("  %s∈ ", symbol_table[symbol_table_indices[i]].id);
        printf("{");
        _inorder_set_traversal(state->component_states[i]->value_set);
        printf("%s", state->component_states[i]->number_of_values ? "\b}" : "}");
      }
    }
    printf("  ]");
  }
}



























void avl_to_dot_file(const char *file, ConcreteValueSet *head) {
  FILE *dot_file = fopen(file, "wt");
  void _traverse(ConcreteValueSet *node) {
    if (!node) return;
    fprintf(dot_file, "\t%ld [label=\"%d\", shape=\"circle\", fillcolor=\"#FFEFD5\"];\n",
      (unsigned long) node, node->value);
    fprintf(dot_file, "\t%ld -> %ld;\n", (unsigned long) node, node->left ? (unsigned long) node->left : (((unsigned long) node) * 1000));
    fprintf(dot_file, "\t%ld -> %ld;\n", (unsigned long) node, node->right ? (unsigned long) node->right : (((unsigned long) node) * 10000));
    if (!node->left)
      fprintf(dot_file, "\t%ld [label=\"NULL\", shape=\"plaintext\", fillcolor=\"#FFEFD5\"];\n", ((unsigned long) node) * 1000);
    if (!node->right)
      fprintf(dot_file, "\t%ld [label=\"NULL\", shape=\"plaintext\", fillcolor=\"#FFEFD5\"];\n", ((unsigned long) node) * 10000);
    _traverse(node->left);
    _traverse(node->right);
    return;
  }

  fprintf(dot_file, "digraph Concrete_Set {\n\n\trankdir=TB;\n\tnode [style=\"filled\"];\n\n");
  _traverse(head);
  fprintf(dot_file, "\n}");
  fclose(dot_file);
  return;
}