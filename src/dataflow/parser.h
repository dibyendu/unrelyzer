#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1000
#define SYMBOL_TABLE_SIZE 200

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
} SymbolTable;

typedef struct ParseTreeT {
  char *token, *printable;
  int is_terminal, number_of_children, visualization_id;
  struct ParseTreeT **children;
} ParseTree;

typedef struct AstT {
  char *token;
  AstType type;
  int number_of_children, line_number, visualization_id;
  struct AstT **children;
} Ast;

typedef struct CfgT {
  int program_point, visualization_id;
  Ast *statement;
  struct CfgT *out_true, *out_false, *in1, *in2;
} Cfg;

char *yytext;
FILE *yyin;
int line_number, max_line_number, parse_stack_top, ast_stack_top, parse_tree_node_id, ast_node_id, meet_node_visualization_id;
SymbolTable *symbol_table;
ParseTree *parse_tree, **parse_stack;
Ast *ast, **ast_stack, ***data_flow_matrix;
Cfg **cfg_node_bucket, *cfg;

//void parse(const char *);
//void generate_dot_file(const char *, GraphT);
//void build_parse_tree(const char *, const char *, int, int);
//void pretty_print_parse_tree(ParseTree *);
//void build_abstract_syntax_tree(const char *, AstType, int, int);
//void build_control_flow_graph(Cfg *, Ast *);
Ast *invert_expression(Ast *);
char *expression_to_string(Ast *, char *);
