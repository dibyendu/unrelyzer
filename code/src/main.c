#include "dataflow/parser.h"
#include "concrete.h"

void evaluate_expression(int from_program_point, int to_program_point, Ast *node) {
  void _operate(const char *operator, ConcreteStateSet **result, ConcreteStateSet *set1, ConcreteStateSet *set2) {
    void _traverse_set1(ConcreteStateSet *node1) {
      void _traverse_set2(ConcreteStateSet *node2) {
        if (!node2) return;
        _traverse_set2(node2->left);
        _traverse_set2(node2->right);
        int new_value;
        double new_probability;
        if (!strcmp(operator, "+")) {
          new_value = node1->state.value + node2->state.value;
          new_probability = PR_ADD;
        }
        else if (!strcmp(operator, "-")) {
          new_value = node1->state.value - node2->state.value;
          new_probability = PR_SUB;
        }
        else if (!strcmp(operator, "*")) {
          new_value = node1->state.value * node2->state.value;
          new_probability = PR_MUL;
        }
        else if (!strcmp(operator, "/")) {
          new_value = node1->state.value / node2->state.value;
          new_probability = PR_DIV;
        }
        else if (!strcmp(operator, "%")) {
          new_value = node1->state.value % node2->state.value;
          new_probability = PR_REM;
        }
        new_probability *= node1->state.probability * node2->state.probability;
        *result = update_concrete_set(*result, new_value, new_probability, 1);
      }
      if (!node1) return;
      _traverse_set1(node1->left);
      _traverse_set1(node1->right);
      _traverse_set2(set2);
      return;
    }
    *result = free_concrete_set((ConcreteStateSet *) *result);
    _traverse_set1(set1);
    return;
  }

  void _evaluate(Ast *node) {
    if (!node->number_of_children) {
      node->value = free_concrete_set((ConcreteStateSet *) node->value);
      if (node->type == NUMBLOCK)
        node->value = update_concrete_set((ConcreteStateSet *) node->value, atoi(node->token), 1.0, 0);
      else if (node->type == IDBLOCK) {
        int index = get_symbol_table_index(node->token);
        if (!symbol_table[index].concrete)
          symbol_table[index].concrete = calloc(N_lines + 1, sizeof(ConcreteStateSet *));
        if (!symbol_table[index].concrete[from_program_point]) {
          int i;
          for (i = MININT; i <= MAXINT; ++i)
            symbol_table[index].concrete[from_program_point] =
            update_concrete_set((ConcreteStateSet *) symbol_table[index].concrete[from_program_point], i, PR_RD, 0);
        }
        node->value = symbol_table[index].concrete[from_program_point];
      }
      return;
    }
    _evaluate(node->children[0]);
    _evaluate(node->children[1]);
    _operate(node->token, (ConcreteStateSet **) &(node->value), (ConcreteStateSet *) node->children[0]->value,
      (ConcreteStateSet *) node->children[1]->value);

    avl_to_dot_file(node->children[0]->token, node->children[0]->value);
    avl_to_dot_file(node->children[1]->token, node->children[1]->value);
    avl_to_dot_file(node->token, node->value);
    return;
  }

  
  if (node->type == ASSIGNBLOCK) {
    _evaluate(node->children[1]);
    int index = get_symbol_table_index(node->children[0]->token);
    if (!symbol_table[index].concrete)
      symbol_table[index].concrete = calloc(N_lines + 1, sizeof(ConcreteStateSet *));
    symbol_table[index].concrete[to_program_point] = node->children[1]->value;
  }
}

int main(int argc, char **argv) {

  
  
  parse(argv[1]);

  //generate_dot_file("parse-tree.dot", PARSE_TREE);
  free_parse_tree(parse_tree);

  build_control_flow_graph(cfg, ast);

  generate_dot_file("ast.dot", AST);
  //generate_dot_file("cfg.dot", CFG);
  
  
  

  generate_dataflow_equations();

  
  print_dataflow_equations();

  /*
    data_flow_matrix[N_lines+1][N_lines+1];
    free(data_flow_matrix);

  */

  int i, j;
  for (i = 0; i <= N_lines; i++) {
    if (data_flow_matrix[i]) {
      for (j = 0; j <= N_lines; j++) {
        if (data_flow_matrix[i][j]) {
          char stmt[200] = {0};
          printf("[%d,%d] -> %s\n", i, j, expression_to_string(data_flow_matrix[i][j], stmt));
          evaluate_expression(j, i, data_flow_matrix[i][j]);
          avl_to_dot_file("x0", symbol_table[get_symbol_table_index("x")].concrete[0]);
          avl_to_dot_file("x1", symbol_table[get_symbol_table_index("x")].concrete[1]);
          avl_to_dot_file("y0", symbol_table[get_symbol_table_index("y")].concrete[0]);
          avl_to_dot_file("y1", symbol_table[get_symbol_table_index("y")].concrete[1]);
          avl_to_dot_file("z0", symbol_table[get_symbol_table_index("z")].concrete[0]);
          avl_to_dot_file("z1", symbol_table[get_symbol_table_index("z")].concrete[1]);
        }
      }
    }
  }
 
  

  free(symbol_table);
  return 0;
}