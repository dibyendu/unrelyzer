#include "dataflow/parser.h"

int main(int argc, char **argv) {

  
  
  parse(argv[1]);

  build_control_flow_graph(cfg, ast);
  
  
  
  generate_dot_file("parse-tree.dot", PARSE_TREE);
  generate_dot_file("ast.dot", AST);
  generate_dot_file("cfg.dot", CFG);
  
  // pretty_print_parse_tree(parse_tree);
  // free(parse_tree);
  

  generate_dataflow_equations();

  
  print_dataflow_equations();

  // free(data_flow_matrix);

  free(symbol_table);
  return 0;
}