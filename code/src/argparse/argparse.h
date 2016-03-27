#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Used by main to communicate with parse_opt. */
typedef struct arguments {
  char *input, *output, *function, **params;
  bool concrete, abstract, widening, parse_tree, ast, cfg, verbose;
  size_t N_iteration, N_params, N_columns;
  long **value_params, **interval_params;
} Arguments;

bool has_interval_param;

#endif