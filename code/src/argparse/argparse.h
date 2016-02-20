#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <argp.h>
#include <stdbool.h>

/* Used by main to communicate with parse_opt. */
typedef struct arguments {
  char *args[1];
  bool concrete, abstract, widening, parse_tree, ast, cfg, verbose;
  char *output;
} Arguments;

#endif