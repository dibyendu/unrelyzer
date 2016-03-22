#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "avl.h"

#define PARSE_STACK_SIZE 1000
#define SYMBOL_TABLE_SIZE 200
#define FUNCTION_TABLE_SIZE 40
#define MAX_UNARY_ARITH_OPERATOR_NESTING 100
#define MAX_FUNCTION_CALL_NESTING 100
#define MAX_CFG_NODES 1000

typedef enum ErrorT {
  MULT_DECL_VAR,
  MULT_DECL_PARAM,
  UN_DECL,
  NO_INIT,
  UN_DEF,
  PARAM_MISMATCH
} ErrorT;

typedef enum GraphT {
  PARSE_TREE,
  AST,
  CFG
} GraphT;

typedef enum AstType {
  FUNCBLOCK     = 0b100000000000,
  CALLBLOCK     = 0b010000000000,
  RETURNBLOCK   = 0b001000000000,
  CONDITION     = 0b000100000000,
  TRUEBLOCK     = 0b000010000000,
  FALSEBLOCK    = 0b000001000000,
  CONTROLBLOCK  = 0b000000100000,
  NUMBLOCK      = 0b000000010000,
  IDBLOCK       = 0b000000001000,
  ARITHOPBLOCK  = 0b000000000100,
  LOGOPBLOCK    = 0b000000000010,
  ASSIGNBLOCK   = 0b000000000001
} AstType;

typedef enum CfgNodeT {
  MEET_NODE = -1,
  EXIT_NODE
} CfgNodeT;

typedef struct SymbolTableT {
  char *id;
  short *decl_scope, *init_scope, *use_scope;
  void **concrete, **abstract;
} SymbolTable;

typedef struct FunctionTableT {
  char *id;
  unsigned short N_params;
} FunctionTable;

typedef struct ParseTreeT {
  char *token, *printable;
  int number_of_children;
  bool is_terminal;
  struct ParseTreeT **children;
} ParseTree;

typedef struct AstT {
  char *token;
  unsigned short type;
  unsigned int number_of_children, line_number, stmt_number;
  struct AstT **children;
  void *value;
  bool _is_dataflow_node;
} Ast;

typedef struct CfgT {
  int program_point;
  Ast *statement;
  struct CfgT *out_true, *out_false, *in1, *in2;
} Cfg;

extern char const *yytext;
extern char *yylval;
FILE *yyin;
bool is_new_stmt;
unsigned int line_number, parse_stack_top, ast_stack_top;
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
FunctionTable *function_table;
IntSet *constant_set;
unsigned short *symbol_table_indices, N_variables, N_lines;
Ast ***data_flow_matrix;
int parse(const char *filename);
void prune_and_rehash_symbol_table(const char *);
Ast *prune_ast(const char *, char **, int , Ast *);
void build_control_flow_graph(Cfg *, Ast *);
void generate_dot_file(const char *filename, GraphT);
void generate_dataflow_equations(void);
void print_dataflow_equations(FILE *);
void free_parse_tree(ParseTree *);
IntSet *avl_tree_insert(IntSet *, int, bool *);

#endif