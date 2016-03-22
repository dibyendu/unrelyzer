#include "argparse/argparse.h"
#include "dataflow/parser.h"
#include "domains/concrete/concrete.h"
#include "domains/abstract/abstract.h"
#include <time.h>

main(int argc, char **argv) {
  Arguments arguments;

  arguments.concrete = arguments.abstract = arguments.widening = arguments.parse_tree =
  arguments.ast = arguments.cfg = arguments.verbose = arguments.output = 0;

  parse_arguments(argc, argv, &arguments);

  FILE *stream = arguments.output ? fopen(arguments.output, "w") : stdout;

  if (parse(arguments.input)) exit(EXIT_FAILURE);

  char *message;
  if (has_error(&message, arguments.function, arguments.input, arguments.N_params)) {
    fprintf(stderr, "%s\n", message);
    free(message);
    exit(EXIT_FAILURE);
  }




  

  prune_and_rehash_symbol_table(arguments.function);




  int i, j;

  

  printf ("Params = ");
  
  for(i = 0; i < arguments.N_params; i++)
    printf ("%s ", arguments.params[i]);
  printf("\n");

  printf("--------------------- symbol table -------------------------\n");
  for (i = 0; i < N_variables; ++i) {
    printf("%s : decl -> ", symbol_table[symbol_table_indices[i]].id);
    for (j = 0; j < FUNCTION_TABLE_SIZE + 1; ++j)
      if (symbol_table[symbol_table_indices[i]].decl_scope[j] != -1)
        printf("%s(%d) ", j == FUNCTION_TABLE_SIZE ? "global" : function_table[j].id, symbol_table[symbol_table_indices[i]].decl_scope[j]);
    printf("| init -> ");
    for (j = 0; j < FUNCTION_TABLE_SIZE + 1; ++j)
      if (symbol_table[symbol_table_indices[i]].init_scope[j] != -1)
        printf("%s(%d) ", j == FUNCTION_TABLE_SIZE ? "global" : function_table[j].id, symbol_table[symbol_table_indices[i]].init_scope[j]);
    printf("| use -> ");
    for (j = 0; j < FUNCTION_TABLE_SIZE + 1; ++j)
      if (symbol_table[symbol_table_indices[i]].use_scope[j] != -1)
        printf("%s(%d) ", j == FUNCTION_TABLE_SIZE ? "global" : function_table[j].id, symbol_table[symbol_table_indices[i]].use_scope[j]);
    printf("\n");
  }


  


  if (arguments.parse_tree) generate_dot_file("parse.dot", PARSE_TREE);
  free_parse_tree(parse_tree);

  if (arguments.ast) generate_dot_file("ast.dot", AST);

  




  ast = prune_ast(arguments.function, arguments.params, arguments.N_params, ast);

  generate_dot_file("new_ast.dot", AST);
  

  

  return;

  //build_control_flow_graph(cfg, ast);

  if (arguments.cfg) generate_dot_file("cfg.dot", CFG);
  
  /*
   * data_flow_matrix[N_lines+1][N_lines+1];
   */
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
    print_concrete_analysis_result(stream);
  }
  if (arguments.abstract) {
  	fprintf(stream, "---------------------- Result of Abstract Analysis (%ld clock ticks) ----------------------\n", abstract_t);
  	print_abstract_analysis_result(stream);
  }

  free_dataflow_equations();
  fclose(stream);

  exit(EXIT_SUCCESS);
}