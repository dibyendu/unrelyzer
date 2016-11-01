#include "argparse/argparse.h"
#include "parser/parser.h"
#include "dataflow/cfg.h"
#include "domains/concrete/concrete.h"
#include "domains/abstract/abstract.h"
#include <time.h>

void generate_dot_file(const char *file, GraphT type, void (*traverse)(void *, FILE *)) {
  FILE *dot_file = fopen(file, "wt");
  fprintf(dot_file, "digraph %s {\n\n\trankdir=TB;\n\tnode [style=\"filled\"];\n\n",
    type == PARSE_TREE ? "Parse_Tree" : type == AST ? "Abstract_Syntax_Tree" : "Control_Flow_Graph");
  if (type == PARSE_TREE)
    traverse((void *) parse_tree, dot_file);
  else if (type == AST)
    traverse((void *) ast, dot_file);
  else {
    traverse((void *) cfg, dot_file);
    free(cfg_node_hash_table);
  }
  fprintf(dot_file, "\n}");
  fclose(dot_file);
  return;
}

void main(int argc, char **argv) {
  Arguments arguments;
  parse_arguments(argc, argv, &arguments);
  FILE *stream = arguments.output ? fopen(arguments.output, "w") : stdout;

  if (parse(arguments.input)) exit(EXIT_FAILURE);
  else init_cfg_data_structures();

  char *message;
  if (has_input_error(&message, arguments.function, arguments.input, arguments.params, arguments.N_params)) {
    fprintf(stderr, "%s\n", message);
    free(message);
    exit(EXIT_FAILURE);
  }

  prune_and_rehash_symbol_table(arguments.function);

  if (arguments.parse_tree) generate_dot_file("parse.dot", PARSE_TREE, traverse_parse_tree);
  free_parse_tree(parse_tree);
  if (arguments.ast) generate_dot_file("ast.dot", AST, traverse_ast);

  ast = prune_ast(arguments.function, arguments.params, arguments.value_params, arguments.interval_params, arguments.N_params, ast);
  free_arguments(&arguments);
  if (arguments.ast) generate_dot_file("ast(pruned).dot", AST, traverse_ast);

  build_control_flow_graph(cfg, ast);

  if (arguments.cfg) generate_dot_file("cfg.dot", CFG, traverse_cfg);
  else free(cfg_node_hash_table);

  generate_dataflow_equations();  
  print_dataflow_equations(stream);

  fprintf(stream, "\n\nIn the following result\t\tm = %d\t&\tM = %d\n\n", MININT, MAXINT);

  clock_t concrete_t, abstract_t, start_t;
  if (arguments.concrete)  {
    start_t = clock();
    concrete_analysis(arguments.verbose, arguments.N_iteration);
    concrete_t = clock() - start_t;
  }
  if (arguments.abstract)  {
    start_t = clock();
    abstract_analysis(arguments.verbose, arguments.N_iteration, arguments.widening);
    abstract_t = clock() - start_t;
  }

  if (arguments.concrete)  {
    fprintf(stream, "---------------------- Result of Concrete Analysis (%ld clock ticks) ----------------------\n", concrete_t);
    print_concrete_analysis_result(stream, arguments.N_columns);
  }
  if (arguments.abstract) {
  	fprintf(stream, "---------------------- Result of Abstract Analysis (%ld clock ticks) ----------------------\n", abstract_t);
  	print_abstract_analysis_result(stream, arguments.N_columns);
  }

  free_dataflow_equations();
  fclose(stream);

  exit(EXIT_SUCCESS);
}