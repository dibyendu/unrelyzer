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
#define STATEMENT_LINE_BUCKET_SIZE 1000
#define MAX_UNARY_ARITH_OPERATOR_NESTING 100
#define MAX_FUNCTION_CALL_NESTING 100

typedef enum ErrorT {
  MULT_DECL_VAR,
  MULT_DECL_PARAM,
  UN_DECL,
  NO_INIT,
  UN_DEF,
  MULT_DEF_FUNC,
  PARAM_MISMATCH
} ErrorT;

typedef enum GraphT {
  PARSE_TREE,
  AST,
  CFG
} GraphT;

typedef enum AstType {
  FUNCBLOCK     = 0b1000000000000,
  CALLBLOCK     = 0b0100000000000,
  RETURNBLOCK   = 0b0010000000000,
  CONDITION     = 0b0001000000000,
  TRUEBLOCK     = 0b0000100000000,
  FALSEBLOCK    = 0b0000010000000,
  CONTROLBLOCK  = 0b0000001000000,
  NUMBLOCK      = 0b0000000100000,
  IDBLOCK       = 0b0000000010000,
  ARITHOPBLOCK  = 0b0000000001000,
  LOGOPBLOCK    = 0b0000000000100,
  ASSIGNBLOCK   = 0b0000000000010,
  INTERVALBLOCK = 0b0000000000001,
} AstType;

typedef enum OperatorType {
  ADD = 1,
  SUB,
  MUL,
  DIV,
  REM,
  NEQ,
  EQ,
  GT,
  GEQ,
  LT,
  LEQ,
  LAND,
  LOR,
  NOT
} OperatorType;

typedef struct SymbolTableT {
  char *id;
  int *decl_scope, *init_scope, *use_scope;
  void **concrete, **abstract;
} SymbolTable;

typedef struct FunctionTableT {
  char *id;
  size_t N_params;
} FunctionTable;

typedef struct ParseTreeT {
  char *token, *printable;
  size_t number_of_children;
  bool is_terminal;
  struct ParseTreeT **children;
} ParseTree;

typedef struct AstT {
  char *token;
  size_t type, operator_type;
  size_t number_of_children, line_number, stmt_number;
  struct AstT **children;
  void *value;
  bool _is_dataflow_node;
} Ast;

extern char const *yytext;
extern char *yylval;
FILE *yyin;
bool is_new_stmt;
size_t line_number, parse_stack_top, ast_stack_top;
ParseTree *parse_tree, **parse_stack;
Ast *ast, **ast_stack;
size_t *stmt_line_map;
IntegerSet *insert_into_constant_set(IntegerSet *, long);

/*
 * Following variables and functions are useful from outside
 */

SymbolTable *symbol_table;
FunctionTable *function_table;
IntegerSet *constant_set;
size_t *symbol_table_indices, N_variables, N_lines, N_stmts;

int parse(const char *filename);
void traverse_parse_tree(void *, FILE *);
void traverse_ast(void *, FILE *);
void free_parse_tree(ParseTree *);
IntegerSet *avl_tree_insert(IntegerSet *, var_t, bool *);
char *print_subsscript(size_t, char *);

#endif