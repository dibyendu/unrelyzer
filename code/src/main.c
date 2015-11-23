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

    if (MAXINT ~ MININT) ~~ 20 then the parameter passed to initialize_first_program_point
    MUST be false in order to avoid segmentation fault.
  */
  printf("----------------------------------------------------------------------------------------\n");

  int i, initial_program_point = initialize_first_program_point(true);

  for (i = 0; i < 7; ++i)
    iterate(initial_program_point);

  

  
  // for (i = 0; i < SYMBOL_TABLE_SIZE; ++i) {
  //   if (symbol_table[i].id && symbol_table[i].concrete) {
  //     for (j = 0; j <= N_lines; ++j) {
  //       if (symbol_table[i].concrete[j]) {
  //         char fname[10];
  //         sprintf(fname, "%s%d.dot", symbol_table[i].id, j);
  //         avl_to_dot_file(fname, ((ConcreteState *) symbol_table[i].concrete[j])->value_set);
  //       }
  //     }
  //   }
  // }














  for (i = 0; i <= N_lines; i++)
    free(data_flow_graph[i]);
  free(data_flow_graph);
  free(empty_state_status);




  free(symbol_table);
  return 0;
}