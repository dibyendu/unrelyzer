#include "abstract.h"

int N_iterations, first_program_point;
bool is_solution_fixed;

static int initialize_first_program_point() {
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
        if (!symbol_table[symbol_table_indices[j]].abstract)
          symbol_table[symbol_table_indices[j]].abstract = calloc(N_lines + 1, sizeof(AbstractState *));
        symbol_table[symbol_table_indices[j]].abstract[i] = calloc(1, sizeof(AbstractState));
        ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->lower = MININT;
        ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->upper = MAXINT;
        ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->probability = 1.0;
      }
      break;
    }
  }
  return i;
}

void free_abstract_state(AbstractState *state) {
  if (state) {
    if (state->component_states) {
      int i;
      for (i = 0; i < N_variables; ++i)
        if (state->component_states[i])
          free(state->component_states[i]);
      free(state->component_states);
    }
    free(state);
  }
}

static void evaluate_arithmatic(Ast *node, AbstractState *state, int program_point) {
  int stack[EVALUATION_STACK_SIZE], top = 0;
  double probability = 1.0;
  bool visited[SYMBOL_TABLE_SIZE] = {false}, is_empty_result = false;

  void _postfix_traversal(Ast *node) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int index = get_symbol_table_index(node->token);
        if (!visited[index])
        	probability *= ((AbstractState *) symbol_table[index].abstract[program_point])->probability /
        		(((AbstractState *) symbol_table[index].abstract[program_point])->upper -
        		((AbstractState *) symbol_table[index].abstract[program_point])->lower + 1);
        visited[index] = true;
        is_empty_result = ((AbstractState *) symbol_table[index].abstract[program_point])->is_empty_interval;
        stack[top++] = ((AbstractState *) symbol_table[index].abstract[program_point])->lower;
        stack[top++] = ((AbstractState *) symbol_table[index].abstract[program_point])->upper;
      }
      else {
      	int value = atoi(node->token);
        stack[top++] = value;
        stack[top++] = value;
      }
      return;
    }
    int j;
    for (j = 0; j < node->number_of_children; ++j)
      _postfix_traversal(node->children[j]);
    int result_lower = 0, result_upper = 0, operand1_lower, operand1_upper, operand2_upper = stack[--top], operand2_lower = stack[--top];
    if (node->number_of_children == 2) {
      operand1_upper = stack[--top];
      operand1_lower = stack[--top];
    }
    if (!strcmp(node->token, "+")) {
      probability *= PR_ARITH(PR_ADD);
      result_lower = operand1_lower + operand2_lower;
      result_upper = operand1_upper + operand2_upper;
    }
    else if (!strcmp(node->token, "-")) {
      probability *= PR_ARITH(PR_SUB);
      result_lower = operand1_lower - operand2_upper;
      result_upper = operand1_upper - operand2_lower;
    }
    else if (!strcmp(node->token, "*") || !strcmp(node->token, "/") || !strcmp(node->token, "%")) {
      int low_low, low_up, up_low, up_up;
      if (!strcmp(node->token, "*")) {
      	probability *= PR_ARITH(PR_MUL);
      	low_low = operand1_lower * operand2_lower;
      	low_up = operand1_lower * operand2_upper;
      	up_low = operand1_upper * operand2_lower;
      	up_up = operand1_upper * operand2_upper;
      }
      else if (!strcmp(node->token, "/")) {
      	probability *= PR_ARITH(PR_DIV);
      	low_low = operand1_lower / operand2_lower;
      	low_up = operand1_lower / operand2_upper;
      	up_low = operand1_upper / operand2_lower;
      	up_up = operand1_upper / operand2_upper;
      }
      else {
      	probability *= PR_ARITH(PR_REM);
      	low_low = operand1_lower % operand2_lower;
      	low_up = operand1_lower % operand2_upper;
      	up_low = operand1_upper % operand2_lower;
      	up_up = operand1_upper % operand2_upper;
      }
      result_lower = result_upper = low_low;
      if (low_up < result_lower) result_lower = low_up;
      if (up_low < result_lower) result_lower = up_low;
      if (up_up < result_lower) result_lower = up_up;
      if (low_up > result_upper) result_upper = low_up;
      if (up_low > result_upper) result_upper = up_low;
      if (up_up > result_upper) result_upper = up_up;
    }
    stack[top++] = result_lower;
    stack[top++] = result_upper;
  }
  _postfix_traversal(node);
  state->upper = limit(stack[--top]);
  state->lower = limit(stack[--top]);
  state->probability *= probability;
  state->is_empty_interval = is_empty_result;
}

void evaluate_logical(Ast *node, AbstractState *state, AbstractAllTuple *list, int N_list, bool *is_true) {
  int stack[EVALUATION_STACK_SIZE], top = 0;
  double probability = 1.0;
  bool is_first_tuple = true;
  void _postfix_traversal(Ast *node, double *probability) {
    int j;
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int index = get_symbol_table_index(node->token);
        for (j = 0; j < N_list; ++j)
          if (list[j].symbol_table_index == index)
            break;
        stack[top++] = list[j].min_value + list[j].current_index;
      }
      else
        stack[top++] = atoi(node->token);
      return;
    }
    for (j = 0; j < node->number_of_children; ++j)
      _postfix_traversal(node->children[j], probability);
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

  int j;

  #ifdef ABSTRACT_DEBUG
  printf("\n....................... evaluation steps (logical) .......................\n");
  for (j = 0; j < N_list; ++j)
    printf("%s%s, ", j == 0 ? "(" : "", symbol_table[list[j].symbol_table_index].id);
  printf("Result%s  ::  ", j ? ")" : "");
  #endif
    
  while (true) {
    probability = 1.0;
    _postfix_traversal(node, &probability);

    #ifdef ABSTRACT_DEBUG
    for (j = 0; j < N_list; ++j)
      printf("%s%d,", j == 0 ? "(" : "", list[j].min_value + list[j].current_index);
    printf("%d%s  ", limit(stack[top-1]), j ? ")" : "");
    #endif

    if (stack[--top]) {
      *is_true = true;
      for (j = 0; j < N_list; ++j) {
        int value = list[j].min_value + list[j].current_index;
        if (is_first_tuple) {
          state->component_states[list[j].symbol_table_indices_list_index]->lower =
          state->component_states[list[j].symbol_table_indices_list_index]->upper = value;
        }
        else {
          if (state->component_states[list[j].symbol_table_indices_list_index]->lower > value)
            state->component_states[list[j].symbol_table_indices_list_index]->lower = value;
          if (state->component_states[list[j].symbol_table_indices_list_index]->upper < value)
            state->component_states[list[j].symbol_table_indices_list_index]->upper = value;
        }
      }
      is_first_tuple = false;
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

  #ifdef ABSTRACT_DEBUG
  printf("\n..........................................................................\n");
  #endif
}

static void populate_tuples_list(Ast *node, AbstractAllTuple *map, int program_point) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  int index = 0, i;

  void _traverse(Ast *node) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int idx = get_symbol_table_index(node->token);
        if (visited[idx]) return;
        visited[idx] = true;
        map[index].symbol_table_index = idx;
        for (i = 0; i < N_variables; ++i) {
          if (symbol_table_indices[i] == idx) {
            map[index].symbol_table_indices_list_index = i;
            break;
          }
        }
        AbstractState *state = (AbstractState *) symbol_table[idx].abstract[program_point];
        map[index].max_values = state ? (state->is_empty_interval ? 0 : state->upper - state->lower + 1) : 0;
        map[index].min_value = state->lower;
        index += 1;
      }
      return;
    }
    int i = node->number_of_children;
    while (i) _traverse(node->children[--i]);
  }
  
  _traverse(node);
}

static void evaluate_expression(Ast *node, int from_program_point) {
  Ast *evaluation_node = node;
  if (node->type == ASSIGNBLOCK)
    evaluation_node = node->children[1];
  free_abstract_state(node->value);
  if (!empty_state_status[from_program_point]) {
    node->value = NULL;
    return;
  }
  int i, N_unique_variables = get_number_of_unique_variables(evaluation_node);
  node->value = calloc(1, sizeof(AbstractState));
  ((AbstractState *) node->value)->probability = 1.0;
  ((AbstractState *) node->value)->component_states = (AbstractState **) calloc(N_variables, sizeof(AbstractState *));
  for (i = 0; i < N_variables; ++i) ((AbstractState *) node->value)->component_states[i] = calloc(1, sizeof(AbstractState));
  for (i = N_unique_variables; i; i--) ((AbstractState *) node->value)->probability *= PR_ARITH(PR_RD);

  if (node->type == LOGOPBLOCK) {
    bool is_true = false;
    AbstractAllTuple *tuples = (AbstractAllTuple *) calloc(N_unique_variables, sizeof(AbstractAllTuple));
    populate_tuples_list(evaluation_node, tuples, from_program_point);

    evaluate_logical(evaluation_node, (AbstractState *) node->value, tuples, N_unique_variables, &is_true);

    bool *no_need_to_be_copied = (bool *) calloc(N_variables, sizeof(bool));
    for (i = 0; i < N_unique_variables; ++i)
      no_need_to_be_copied[tuples[i].symbol_table_indices_list_index] = true;
    for (i = 0; i < N_variables; ++i) {
      AbstractState *src = (AbstractState *) symbol_table[symbol_table_indices[i]].abstract[from_program_point];
      if (no_need_to_be_copied[i]) {
      	((AbstractState *) node->value)->component_states[i]->probability =
      	src->probability * ((AbstractState *) node->value)->probability / (src->upper - src->lower + 1);
        ((AbstractState *) node->value)->component_states[i]->probability *=
        ((AbstractState *) node->value)->component_states[i]->upper - ((AbstractState *) node->value)->component_states[i]->lower + 1;
        ((AbstractState *) node->value)->component_states[i]->probability =
        ((AbstractState *) node->value)->component_states[i]->probability > 1 ? 1 : ((AbstractState *) node->value)->component_states[i]->probability;
      }
      else {
        ((AbstractState *) node->value)->component_states[i]->lower = src->lower;
        ((AbstractState *) node->value)->component_states[i]->upper = src->upper;
        ((AbstractState *) node->value)->component_states[i]->probability = src->probability * ((AbstractState *) node->value)->probability;
        ((AbstractState *) node->value)->component_states[i]->is_empty_interval = src->is_empty_interval;
      }
    }
    if (!is_true)
      for (i = 0; i < N_variables; ++i)
      	((AbstractState *) node->value)->component_states[i]->is_empty_interval = true;
    free(tuples);
  }
  else if (node->type == ASSIGNBLOCK) {
  	evaluate_arithmatic(evaluation_node, (AbstractState *) node->value, from_program_point);

  	#ifdef ABSTRACT_DEBUG
  	printf("\n....................... evaluation steps (assignment) .......................\n");
  	printf("%s : [%d,%d]\n", node->children[0]->token, ((AbstractState *) node->value)->lower, ((AbstractState *) node->value)->upper);
    printf(".............................................................................\n");
    #endif

    int i, dest_index = -1, index = get_symbol_table_index(node->children[0]->token);
    while (dest_index < N_variables && symbol_table_indices[++dest_index] != index);
    for (i = 0; i < N_variables; ++i) {
      if (i != dest_index) {
      	AbstractState *src = (AbstractState *) symbol_table[symbol_table_indices[i]].abstract[from_program_point];
        ((AbstractState *) node->value)->component_states[i]->lower = src->lower;
        ((AbstractState *) node->value)->component_states[i]->upper = src->upper;
        ((AbstractState *) node->value)->component_states[i]->probability = src->probability;
        ((AbstractState *) node->value)->component_states[i]->is_empty_interval = src->is_empty_interval;
      }
      else {
      	((AbstractState *) node->value)->component_states[i]->lower = ((AbstractState *) node->value)->lower;
        ((AbstractState *) node->value)->component_states[i]->upper = ((AbstractState *) node->value)->upper;
        ((AbstractState *) node->value)->component_states[i]->probability = PR_ARITH(PR_WR) *
        		((AbstractState *) node->value)->probability * (((AbstractState *) node->value)->upper - ((AbstractState *) node->value)->lower + 1);
        ((AbstractState *) node->value)->component_states[i]->probability =
        		((AbstractState *) node->value)->component_states[i]->probability > 1 ? 1 : ((AbstractState *) node->value)->component_states[i]->probability;
        ((AbstractState *) node->value)->component_states[i]->is_empty_interval = ((AbstractState *) node->value)->is_empty_interval;
      }
    }
  }
}

void print_abstract_state(AbstractState *state) {
  if (!state) return;
  if (state->component_states) {
    int i;
    for (i = 0; i < N_variables; ++i) {
      if (state->component_states[i]) {
      	if (!state->component_states[i]->is_empty_interval)
      	  printf("    %s=<[%d,%d], %.12lg>", symbol_table[symbol_table_indices[i]].id, state->component_states[i]->lower,
      		state->component_states[i]->upper, state->component_states[i]->probability);
      	else
      	  printf("    %s=<[], 1>", symbol_table[symbol_table_indices[i]].id);
      }
    }
  }
}

static void upper_bound(AbstractState **state1, AbstractState *state2) {
  if (!state2) return;
  if (!(*state1)) {
    *state1 = state2;
    return;
  }
  int i;
  for (i = 0; i < N_variables; ++i) {
  	if ((*state1)->component_states[i]->is_empty_interval) {
  	  (*state1)->component_states[i]->lower = state2->component_states[i]->lower;
  	  (*state1)->component_states[i]->upper = state2->component_states[i]->upper;
  	  (*state1)->component_states[i]->is_empty_interval = state2->component_states[i]->is_empty_interval;
  	  (*state1)->component_states[i]->probability = state2->component_states[i]->probability;
  	}
  	else if (!state2->component_states[i]->is_empty_interval) {
  	  double pmf1 = (*state1)->component_states[i]->probability / ((*state1)->component_states[i]->upper - (*state1)->component_states[i]->lower + 1),
  	  			 pmf2 = state2->component_states[i]->probability / (state2->component_states[i]->upper - state2->component_states[i]->lower + 1);
  	  if ((*state1)->component_states[i]->lower > state2->component_states[i]->lower)
  	    (*state1)->component_states[i]->lower = state2->component_states[i]->lower;
  	  if ((*state1)->component_states[i]->upper < state2->component_states[i]->upper)
  	    (*state1)->component_states[i]->upper = state2->component_states[i]->upper;
  	  (*state1)->component_states[i]->probability = 1;
  	  if ((*state1)->component_states[i]->probability > pmf1 * ((*state1)->component_states[i]->upper - (*state1)->component_states[i]->lower + 1))
  	    (*state1)->component_states[i]->probability = pmf1 * ((*state1)->component_states[i]->upper - (*state1)->component_states[i]->lower + 1);
  	  if ((*state1)->component_states[i]->probability > pmf2 * ((*state1)->component_states[i]->upper - (*state1)->component_states[i]->lower + 1))
  	    (*state1)->component_states[i]->probability = pmf2 * ((*state1)->component_states[i]->upper - (*state1)->component_states[i]->lower + 1);
  	  (*state1)->component_states[i]->is_empty_interval = false;
  	}
  }
  free_abstract_state(state2);
}

bool is_same_interval(AbstractState *state1, AbstractState *state2) {
  if (!state1 && !state2) return true;
  if (!state1 || !state2) return false;
  if (state1->is_empty_interval && state2->is_empty_interval) return true;
  if (state1->is_empty_interval || state2->is_empty_interval) return false;
  if (state1->lower == state2->lower && state1->upper == state2->upper) return true;
  return false;
}

bool is_less_than_equal_to(AbstractState *state1, AbstractState *state2) {
  if (state1->is_empty_interval) return true;
  if (state2->is_empty_interval) return false;
  if (state1->lower < state2->lower || state1->upper > state2->upper) return false;
  if (state1->probability / (state1->upper - state1->lower + 1) >= state2->probability / (state2->upper - state2->lower + 1))
  	return true;
  return false;
}

void find_max_in_constant_set(IntSet *set, int key, int *max) {
	if (!set) return;
	find_max_in_constant_set(set->left, key, max);
	if (set->value <= key) *max = set->value;
	find_max_in_constant_set(set->right, key, max);
}

void find_min_in_constant_set(IntSet *set, int key, int *min) {
	if (!set) return;
	find_min_in_constant_set(set->right, key, min);
	if (set->value >= key) *min = set->value;
	find_min_in_constant_set(set->left, key, min);
}

static bool iterate(int initial_program_point, bool widening) {

  bool has_fixed_point_reached = true;
  void _iterate(bool iterate_exit_program_point) {
    int i, j, k, start = iterate_exit_program_point ? 0 : initial_program_point + 1, end = iterate_exit_program_point ? 0 : N_lines;
    for (i = start; i <= end; i++) {
      bool is_first_eqn = true, program_point_is_fixed = true;
      AbstractState *previous_state = NULL;

      #ifdef ABSTRACT_DEBUG
      printf("\n==============================================================================\n");
      #endif

      for (j = 0; j <= N_lines; j++) {
        if (data_flow_graph[i][j]) {

          #ifdef ABSTRACT_DEBUG
          char stmt[200] = {0};
          printf("G_%d <- G_%d [ %s ] %s\n", i ? i : N_lines + 1, j ? j : N_lines + 1, expression_to_string(data_flow_matrix[i][j], stmt),
            data_flow_matrix[i][j]->type == LOGOPBLOCK ? "logical" : (data_flow_matrix[i][j]->type == ASSIGNBLOCK ? "assignment" : ""));
          #endif

          if (is_first_eqn) {
            is_first_eqn = false;
            evaluate_expression(data_flow_matrix[i][j], j);

            #ifdef ABSTRACT_DEBUG
            printf("evaluated result :: \t");
            print_abstract_state((AbstractState *) data_flow_matrix[i][j]->value);
            printf("\n");
            #endif

            previous_state = (AbstractState *) data_flow_matrix[i][j]->value;
            data_flow_matrix[i][j]->value = NULL;
            continue;
          }
          evaluate_expression(data_flow_matrix[i][j], j);

          #ifdef ABSTRACT_DEBUG
          printf("evaluated result ::\t");
          print_abstract_state((AbstractState *) data_flow_matrix[i][j]->value);
          #endif

          upper_bound(&previous_state, (AbstractState *) data_flow_matrix[i][j]->value);
          data_flow_matrix[i][j]->value = NULL;

          #ifdef ABSTRACT_DEBUG
          printf("\nupper bound ::\t");
          print_abstract_state(previous_state);
          printf("\n");
          #endif
        }
      }
      if (!is_first_eqn) {
        empty_state_status[i] = previous_state ? true : false;
        for (k = 0; k < N_variables; ++k) {
          bool is_fixed = is_same_interval(symbol_table[symbol_table_indices[k]].abstract[i], previous_state ? previous_state->component_states[k] : NULL);
          
          #ifdef ABSTRACT_DEBUG
          printf("%s is %s\t", symbol_table[symbol_table_indices[k]].id, is_fixed ? "fixed" : "NOT fixed");
          #endif

          if (!is_fixed) {
          	if (widening && symbol_table[symbol_table_indices[k]].abstract[i]) {
          		if (!is_less_than_equal_to(previous_state->component_states[k], symbol_table[symbol_table_indices[k]].abstract[i])) {

          			#ifdef ABSTRACT_DEBUG
          			printf("--\tapplying widening on ::  ");
          			if (((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->is_empty_interval)
          				printf("<[], 1> and ");
          			else
	          			printf("<[%d,%d], %.8lg> and ", ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->lower,
	          				((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->upper,
	          				((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->probability);
	          		if (previous_state->component_states[k]->is_empty_interval)
	          			printf("<[], 1>");
	          		else
	          			printf("<[%d,%d], %.8lg>", previous_state->component_states[k]->lower, previous_state->component_states[k]->upper,
	          				previous_state->component_states[k]->probability);
          			printf(" = ");
          			#endif

          			if (((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->is_empty_interval) {
          				free_abstract_state(symbol_table[symbol_table_indices[k]].abstract[i]);
            			symbol_table[symbol_table_indices[k]].abstract[i] = previous_state->component_states[k];
            			is_fixed = previous_state->component_states[k]->is_empty_interval ? true : is_fixed;
          			}
          			else if (!previous_state->component_states[k]->is_empty_interval) {
          				AbstractState *state = (AbstractState *) calloc(1, sizeof(AbstractState));
            			find_max_in_constant_set(constant_set, previous_state->component_states[k]->lower, &(state->lower));
            			find_min_in_constant_set(constant_set, previous_state->component_states[k]->upper, &(state->upper));
            			double pmf_cur  = previous_state->component_states[k]->probability /
            												(previous_state->component_states[k]->upper - previous_state->component_states[k]->lower + 1),
            						 pmf_prev = ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->probability /
            						 						(((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->upper -
            						 						 ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->lower + 1);
            			state->probability = (pmf_cur > pmf_prev ? pmf_cur : pmf_prev) * (state->upper - state->lower + 1);
									state->probability = state->probability > 1.0 ? 1.0 : state->probability;
									is_fixed = is_same_interval(symbol_table[symbol_table_indices[k]].abstract[i], state);
									free_abstract_state(symbol_table[symbol_table_indices[k]].abstract[i]);
            			symbol_table[symbol_table_indices[k]].abstract[i] = state;
          			}

            		#ifdef ABSTRACT_DEBUG
          			if (((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->is_empty_interval)
          				printf("<[], 1>");
          			else
	          			printf("<[%d,%d], %.8lg>", ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->lower,
	          				((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->upper,
	          				((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->probability);
          			printf("\n");
          			#endif

          		}
          		else
          			is_fixed = true;
          	}
          	else {
          		free_abstract_state(symbol_table[symbol_table_indices[k]].abstract[i]);
            	symbol_table[symbol_table_indices[k]].abstract[i] = previous_state ? previous_state->component_states[k] : NULL;
          	}
          }
          else if (previous_state)
            free_abstract_state(previous_state->component_states[k]);
          program_point_is_fixed &= is_fixed;
        }

        #ifdef ABSTRACT_DEBUG
        printf("\nG_%d (%s) ::\t", i ? i : N_lines + 1, program_point_is_fixed ? "fixed" : "NOT fixed");
        for (k = 0; k < N_variables; ++k) {
          printf("  %s=<[", symbol_table[symbol_table_indices[k]].id);
          if (((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->is_empty_interval)
            printf("], 1>");
          else
          	printf("%d,%d], %.8lg>", ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->lower,
              ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->upper,
              ((AbstractState *) symbol_table[symbol_table_indices[k]].abstract[i])->probability);
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

void abstract_analysis(bool widening) {

	constant_set = insert_into_constant_set(constant_set, MININT);
  constant_set = insert_into_constant_set(constant_set, MAXINT);
  int i = 0, initial_program_point = initialize_first_program_point();
  bool is_fixed;
  
  do {
    i += 1;
    
    #ifdef ABSTRACT_DEBUG
    printf("\t\t\t\titeration-%d", i);
    #endif

    is_fixed = iterate(initial_program_point, widening);
  } while(!is_fixed && (widening ? true : i < MAX_ITERATION));

  // Narrowing
  if (widening) iterate(initial_program_point, false);

  is_solution_fixed = is_fixed;
  N_iterations = i;
  first_program_point = initial_program_point;

  free(empty_state_status);
  for (i = 0; i <= N_lines; i++)
    free(data_flow_graph[i]);
  free(data_flow_graph);
}

void print_abstract_analysis_result() {
  int i;
  if (is_solution_fixed) printf("Reached fixed point after %d iterations\n\n", N_iterations);
  else printf("Fixed point is not reached within %d iterations\n\n", N_iterations);

  for (i = 1; i <= N_lines; i++) {
    if (data_flow_matrix[i]) {
      printf("G_%d  ::  ", i);
      int j;
      for (j = 0; j < N_variables; ++j) {
        printf("    %s=<[", symbol_table[symbol_table_indices[j]].id);
        if (i == first_program_point)
          printf("m,M], 1>");
        else if (((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->is_empty_interval)
          printf("], 1>");
        else {
        	char low[40], up[40];
        	sprintf(low, "%d", ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->lower);
        	sprintf(up, "%d", ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->upper);
          printf("%s,%s], %.12lg>", ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->lower == MININT ? "m" :
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->lower == MAXINT ? "M" : low,
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->upper == MININT ? "m" : 
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->upper == MAXINT ? "M" : up,
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[i])->probability);
        }
      }
      printf("\n");
    }
  }
  if (data_flow_matrix[0]) {
    printf("G_%d  ::  ", N_lines + 1);
    int j;
    for (j = 0; j < N_variables; ++j) {
      printf("    %s=<[", symbol_table[symbol_table_indices[j]].id);
      if (((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->is_empty_interval)
        printf("], 1>");
      else {
      	char low[40], up[40];
        sprintf(low, "%d", ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->lower);
        sprintf(up, "%d", ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->upper);
        printf("%s,%s], %.12lg>", ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->lower == MININT ? "m" :
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->lower == MAXINT ? "M" : low,
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->upper == MININT ? "m" : 
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->upper == MAXINT ? "M" : up,
            ((AbstractState *) symbol_table[symbol_table_indices[j]].abstract[0])->probability);
      }
    }
    printf("\n");
  }
}