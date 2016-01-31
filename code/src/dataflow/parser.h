#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "avl.h"

#define STACK_SIZE 1000
#define SYMBOL_TABLE_SIZE 200
#define MAX_CFG_NODES 1000

typedef enum GraphT {
  PARSE_TREE,
  AST,
  CFG
} GraphT;

typedef enum AstType {
  CONDITION = 4,
  TRUEBLOCK,
  FALSEBLOCK,
  CONTROLBLOCK,
  NUMBLOCK,
  IDBLOCK,
  ARITHOPBLOCK,
  LOGOPBLOCK,
  ASSIGNBLOCK
} AstType;

typedef enum CfgNodeT {
  MEET_NODE = -1,
  EXIT_NODE
} CfgNodeT;

typedef struct SymbolTableT {
  char *id;
  void **concrete;
  void **abstract;
} SymbolTable;

typedef struct ParseTreeT {
  char *token, *printable;
  int is_terminal, number_of_children;
  struct ParseTreeT **children;
} ParseTree;

typedef struct AstT {
  char *token;
  AstType type;
  int number_of_children, line_number;
  struct AstT **children;
  void *value;
  bool _is_dataflow_node;
} Ast;

typedef struct CfgT {
  int program_point;
  Ast *statement;
  struct CfgT *out_true, *out_false, *in1, *in2;
} Cfg;

char *yytext;
FILE *yyin;
bool is_negative_number;
int line_number, parse_stack_top, ast_stack_top;
ParseTree *parse_tree, **parse_stack;
Ast *ast, **ast_stack;
Cfg **cfg_node_bucket, *cfg;
Ast *invert_expression(Ast *);
char *expression_to_string(Ast *, char *);
IntSet *insert_into_constant_set(IntSet *, int);

/*
 * Following variables and functions are useful from outside
 */

SymbolTable *symbol_table;
IntSet *constant_set;
unsigned short *symbol_table_indices, N_variables, N_lines;
Ast ***data_flow_matrix;
void parse(const char *filename);
void build_control_flow_graph(Cfg *, Ast *);
void generate_dot_file(const char *filename, GraphT);
void generate_dataflow_equations(void);
void print_dataflow_equations(void);
void free_parse_tree(ParseTree *);
IntSet *avl_tree_insert(IntSet *, int, bool *);

#endif