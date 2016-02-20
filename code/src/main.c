#include "argparse/argparse.h"
#include "dataflow/parser.h"
#include "domains/concrete/concrete.h"
#include "domains/abstract/abstract.h"
#include <time.h>

int main(int argc, char **argv) {
  Arguments arguments;

  arguments.concrete = arguments.abstract = arguments.widening = arguments.parse_tree = arguments.ast = arguments.cfg = arguments.verbose = 0;
  arguments.output = NULL;

  parse_arguments(argc, argv, &arguments);

  FILE *stream = stdout;
  if (arguments.output) stream = fopen(arguments.output, "w");

  parse(arguments.args[0]);

  if (arguments.parse_tree) generate_dot_file("parse-tree.dot", PARSE_TREE);
  free_parse_tree(parse_tree);

  build_control_flow_graph(cfg, ast);

  if (arguments.ast) generate_dot_file("ast.dot", AST);
  if (arguments.cfg) generate_dot_file("cfg.dot", CFG);
  
  // data_flow_matrix[N_lines+1][N_lines+1];
  generate_dataflow_equations();  
  print_dataflow_equations(stream);

  fprintf(stream, "\n\nIn the following result\t\tm = %d\t&\tM = %d\n\n", MININT, MAXINT);

  clock_t concrete_t, abstract_t, start_t;
  if (arguments.concrete)  {
    start_t = clock();
    concrete_analysis(arguments.verbose);
    concrete_t = clock() - start_t;
  }
  if (arguments.abstract)  {
    start_t = clock();
    abstract_analysis(arguments.verbose, arguments.widening);
    abstract_t = clock() - start_t;
  }

  if (arguments.concrete)  {
    fprintf(stream, "---------------------- Result of Concrete Analysis (%ld clock ticks) ----------------------\n", concrete_t);
    print_concrete_analysis_result(stream);
  }
  if (arguments.abstract) {
  	fprintf(stream, "---------------------- Result of Abstract Analysis (%ld clock ticks) ----------------------\n", abstract_t);
  	print_abstract_analysis_result(stream);
  }

  free_dataflow_equations();

  fclose(stream);

  return 0;
}