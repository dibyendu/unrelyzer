#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <argp.h>
#include <stdlib.h>
#include <stdbool.h>

/* Used by main to communicate with parse_opt. */
typedef struct arguments {
  char *input, *output, *function, **params;
  bool concrete, abstract, widening, parse_tree, ast, cfg, verbose;
  unsigned int N_iteration, N_params;
} Arguments;

#endif