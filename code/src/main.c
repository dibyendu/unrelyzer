#include "dataflow/parser.h"
#include "domains/concrete/concrete.h"
#include "domains/abstract/abstract.h"
#include <time.h>

int main(int argc, char **argv) {

  bool arg_concrete, arg_widening, arg_parse_tree, arg_ast, arg_cfg;
  arg_concrete = arg_widening = arg_parse_tree = arg_ast = arg_cfg = false;

  if (argc > 2) {
    int i;
    for (i = 1; i < argc - 1; ++i) {
      if (!strcmp("-c", argv[i]) || !strcmp("--concrete", argv[i])) arg_concrete = true;
      if (!strcmp("-w", argv[i]) || !strcmp("--widening", argv[i])) arg_widening = true;
      if (!strcmp("-p", argv[i]) || !strcmp("--parse-tree", argv[i])) arg_parse_tree = true;
      if (!strcmp("-a", argv[i]) || !strcmp("--ast", argv[i])) arg_ast = true;
      if (!strcmp("-f", argv[i]) || !strcmp("--cfg", argv[i])) arg_cfg = true;
    }
  }

  parse(argv[argc - 1]);

  if (arg_parse_tree) generate_dot_file("parse-tree.dot", PARSE_TREE);
  free_parse_tree(parse_tree);

  build_control_flow_graph(cfg, ast);

  if (arg_ast) generate_dot_file("ast.dot", AST);
  if (arg_cfg) generate_dot_file("cfg.dot", CFG);
  
  generate_dataflow_equations();  
  print_dataflow_equations();

  printf("\n\nIn the following result\t\tm = %d\t&\tM = %d\n\n", MININT, MAXINT);

  clock_t concrete_t, abstract_t, start_t;
  if (arg_concrete)  {
    start_t = clock();
    concrete_analysis();
    concrete_t = clock() - start_t;
  }
  start_t = clock();
  abstract_analysis(arg_widening);
  abstract_t = clock() - start_t;

  if (arg_concrete)  {
    printf("---------------------- Result of Concrete Analysis (%ld clock ticks) ----------------------\n", concrete_t);
    print_concrete_analysis_result();
  }
  printf("---------------------- Result of Abstract Analysis (%ld clock ticks) ----------------------\n", abstract_t);
  print_abstract_analysis_result();  

  /*
    data_flow_matrix[N_lines+1][N_lines+1];
    free(data_flow_matrix);
  */

  free(symbol_table);
  free_constant_set(constant_set);
  return 0;
}