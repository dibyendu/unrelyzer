#include "dataflow/parser.h"
#include "concrete.h"




int main(int argc, char **argv) {

  
  
  parse(argv[1]);

  //generate_dot_file("parse-tree.dot", PARSE_TREE);
  free_parse_tree(parse_tree);

  build_control_flow_graph(cfg, ast);

  generate_dot_file("ast.dot", AST);
  generate_dot_file("cfg.dot", CFG);
  
  
  

  generate_dataflow_equations();

  
  print_dataflow_equations();

  /*
    data_flow_matrix[N_lines+1][N_lines+1];
    free(data_flow_matrix);

  */

  initialize_first_program_point();

  int i, j;
  for (i = 1; i <= N_lines; i++) {
    if (data_flow_matrix[i]) {
      for (j = 0; j <= N_lines; j++) {
        if (data_flow_matrix[i][j]) {
          char stmt[200] = {0};
          printf("%s\n", expression_to_string(data_flow_matrix[i][j], stmt));
          evaluate_expression(data_flow_matrix[i][j], j);
          printf("=====================================================================\n");
          print_concrete_state((ConcreteState *) data_flow_matrix[i][j]->value);
          printf("\n=====================================================================\n");
        }
      }
    }
  }
  i = 0;
  for (j = 0; j <= N_lines; j++) {
    if (data_flow_matrix[i][j]) {
      char stmt[200] = {0};
      printf("%s\n", expression_to_string(data_flow_matrix[i][j], stmt));
      evaluate_expression(data_flow_matrix[i][j], j);
      printf("=====================================================================\n");
      print_concrete_state((ConcreteState *) data_flow_matrix[i][j]->value);
      printf("\n=====================================================================\n");
    }
  }
  for (i = 0; i < SYMBOL_TABLE_SIZE; ++i) {
    if (symbol_table[i].id && symbol_table[i].concrete) {
      for (j = 0; j <= N_lines; ++j) {
        if (symbol_table[i].concrete[j]) {
          char fname[10];
          sprintf(fname, "%s%d.dot", symbol_table[i].id, j);
          avl_to_dot_file(fname, ((ConcreteState *) symbol_table[i].concrete[j])->value_set);
        }
      }
    }
  }


  free(symbol_table);
  return 0;
}