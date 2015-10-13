#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1000

int yylex();
int yyparse();
char *yytext;
FILE *yyin;

typedef enum GRAPH_TYPE {
  PARSE_TREE,
  AST,
  CFG
} GRAPH_TYPE;

typedef enum AST_NODE_TYPE {
  CONDITION = 4,
  TRUEBLOCK,
  FALSEBLOCK,
  CONTROLBLOCK,
  NUMBLOCK,
  IDBLOCK,
  ARITHOPBLOCK,
  LOGOPBLOCK,
  ASSIGNBLOCK
} AST_NODE_TYPE;

typedef enum CFG_NODE_TYPE {
  MEET_NODE = -1
} CFG_NODE_TYPE;

typedef struct _SYMBOL_TABLE_NODE {
  char *id;
  struct _SYMBOL_TABLE_NODE *next;
} SYMBOL_TABLE_NODE;

typedef struct _PARSE_TREE_NODE {
  char *token, *printable;
  short is_terminal, number_of_children, visualization_id;
  struct _PARSE_TREE_NODE **children;
} PARSE_TREE_NODE;

typedef struct _AST_NODE {
  char *token;
  AST_NODE_TYPE type;
  short number_of_children, line_number, visualization_id;
  struct _AST_NODE **children;
} AST_NODE;

typedef struct _CFG_NODE {
  short program_point, visualization_id;
  AST_NODE *statement;
  struct _CFG_NODE *out_true, *out_false, *in1, *in2;
} CFG_NODE;

void store_symbols(const char *);
void gen_parse_tree(const char *, const char *, short, short);
void pretty_print_parse_tree(PARSE_TREE_NODE *);
void gen_ast(const char *, AST_NODE_TYPE, short, short);
void gen_cfg(CFG_NODE *, AST_NODE *);
AST_NODE *make_expression_false(AST_NODE *);
char *expression_to_string(AST_NODE *, char *);
 
short line_number, max_line_number, parse_stack_top, ast_stack_top, parse_tree_node_id, ast_node_id, meet_node_visualization_id;
SYMBOL_TABLE_NODE *symbol_table;
PARSE_TREE_NODE *parse_tree, **parse_stack;
AST_NODE *ast, **ast_stack, ***data_flow_matrix;
CFG_NODE **cfg_node_bucket, *cfg;