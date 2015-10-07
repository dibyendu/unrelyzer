%{
#include "parser.h"
%}
%start program
%token SEMICOLON
%token COMMA
%token ASSIGN
%token IF
%token ELSE
%token WHILE
%token UNILOGOP
%token BILOGOP1
%token BILOGOP2
%token POPEN
%token PCLOSE
%token BOPEN
%token BCLOSE
%token UNIARITHOP
%token BIARITHOP1
%token BIARITHOP2
%token ID
%token NUM
%token INT
%% 
program   :   compstmt    { gen_parse_tree("program", NULL, 0, 1); gen_ast("program", 0, ast_stack_top, 0); } ;
compstmt  :   stmt
              compstmt    { gen_parse_tree("compstmt", NULL, 0, 2); }
            | stmt        { gen_parse_tree("compstmt", NULL, 0, 1); } ;
stmt      :   type
              ID          { store_symbols(yytext); gen_parse_tree("ID", yytext, 1, 0); }
              idlist
              SEMICOLON   { gen_parse_tree("SEMICOLON", ";", 1, 0); gen_parse_tree("stmt", NULL, 0, 4); }
            | ID          { store_symbols(yytext); gen_parse_tree("ID", yytext, 1, 0); gen_ast(yytext, IDBLOCK, 0, 0); }
              ASSIGN      { gen_parse_tree("ASSIGN", "=", 1, 0); }
              expr        { gen_ast("=", ASSIGNBLOCK, 2, line_number + 1); }
              SEMICOLON   { gen_parse_tree("SEMICOLON", ";", 1, 0); gen_parse_tree("stmt", NULL, 0, 4); }
            | IF          { gen_parse_tree("IF", "if", 1, 0); }
              POPEN       { gen_parse_tree("POPEN", "(", 1, 0); }
              condition   { ast_stack[ast_stack_top - 1]->type = CONDITION;
                            ast_stack[ast_stack_top - 1]->line_number = line_number + 1;
                          }
              PCLOSE      { gen_parse_tree("PCLOSE", ")", 1, 0); }
              stmtblock   { unsigned int stmt_count = ast_stack_top - 1;
                            while (ast_stack[stmt_count--]->type != CONDITION);
                            gen_ast("true\nblock", TRUEBLOCK, ast_stack_top - stmt_count - 2, 0);
                          }
              elsestmt    { gen_parse_tree("stmt", NULL, 0, 6);
                            unsigned short cond_block_offset = ast_stack[ast_stack_top - 1]->type == TRUEBLOCK ? 2 : 3,
                                           stmt_line_number = ast_stack[ast_stack_top - cond_block_offset]->line_number;
                            ast_stack[ast_stack_top - cond_block_offset]->line_number = 0;
                            ast_stack[ast_stack_top - cond_block_offset]->type = LOGOPBLOCK;
                            gen_ast("if", CONTROLBLOCK, cond_block_offset, stmt_line_number);
                          }
            | WHILE       { gen_parse_tree("WHILE", "while", 1, 0); }
              POPEN       { gen_parse_tree("POPEN", "(", 1, 0); }
              condition   { ast_stack[ast_stack_top - 1]->type = CONDITION;
                            ast_stack[ast_stack_top - 1]->line_number = line_number + 1;
                          }
              PCLOSE      { gen_parse_tree("PCLOSE", ")", 1, 0); }
              stmtblock   { gen_parse_tree("stmt", NULL, 0, 5);
                            unsigned int stmt_count = ast_stack_top - 1;
                            while (ast_stack[stmt_count--]->type != CONDITION);
                            gen_ast("true\nblock", TRUEBLOCK, ast_stack_top - stmt_count - 2, 0);
                            unsigned short stmt_line_number = ast_stack[ast_stack_top - 2]->line_number;
                            ast_stack[ast_stack_top - 2]->line_number = 0;
                            ast_stack[ast_stack_top - 2]->type = LOGOPBLOCK;
                            gen_ast("while", CONTROLBLOCK, 2, stmt_line_number);
                          } ;
type      :   INT         { gen_parse_tree("INT", yytext, 1, 0); gen_parse_tree("type", NULL, 0, 1); } ;
idlist    :   /* empty */ { gen_parse_tree("EPSILON", "e", 1, 0); gen_parse_tree("idlist", NULL, 0, 1); }
            | COMMA       { gen_parse_tree("COMMA", ",", 1, 0); }
              ID          { store_symbols(yytext); gen_parse_tree("ID", yytext, 1, 0); }
              idlist      { gen_parse_tree("idlist", NULL, 0, 3); } ;
elsestmt  :   /* empty */ { gen_parse_tree("EPSILON", "e", 1, 0); gen_parse_tree("elsestmt", NULL, 0, 1); }
            | ELSE        { gen_parse_tree("ELSE", "else", 1, 0); }
              stmtblock   { gen_parse_tree("elsestmt", NULL, 0, 2);
                            unsigned int stmt_count = ast_stack_top - 1;
                            while (ast_stack[stmt_count--]->type != TRUEBLOCK);
                            gen_ast("false\nblock", FALSEBLOCK, ast_stack_top - stmt_count - 2, 0);
                          } ;
stmtblock :   BOPEN       { gen_parse_tree("BOPEN", "{", 1, 0); }
              compstmt
              BCLOSE      { gen_parse_tree("BCLOSE", "}", 1, 0); gen_parse_tree("stmtblock", NULL, 0, 3); }
            | stmt        { gen_parse_tree("stmtblock", NULL, 0, 1); } ;
condition :   condition
              BILOGOP1    { gen_parse_tree("BILOGOP", yytext, 1, 0); gen_ast(yytext, LOGOPBLOCK, 0, 0); }
              condterm    { gen_parse_tree("condition", NULL, 0, 3);
                            AST_NODE *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            gen_ast(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | condterm    { gen_parse_tree("condition", NULL, 0, 1); } ;
condterm  :   condterm
              BILOGOP2    { gen_parse_tree("BILOGOP", yytext, 1, 0); gen_ast(yytext, LOGOPBLOCK, 0, 0); }
              condfactor  { gen_parse_tree("condterm", NULL, 0, 3);
                            AST_NODE *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            gen_ast(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | condfactor  { gen_parse_tree("condterm", NULL, 0, 1); } ;
condfactor :  expr        { gen_parse_tree("condfactor", NULL, 0, 1); }
            | UNILOGOP    { gen_parse_tree("UNILOGOP", yytext, 1, 0); gen_ast(yytext, LOGOPBLOCK, 0, 0); }
              expr        { gen_parse_tree("condfactor", NULL, 0, 2);
                            AST_NODE *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            gen_ast(tmp->token, tmp->type, 1, 0);
                            free(tmp->token);
                            free(tmp);
                          } ;
expr      :   expr
              BIARITHOP1  { gen_parse_tree("BIARITHOP", yytext, 1, 0); gen_ast(yytext, ARITHOPBLOCK, 0, 0); }
              term        { gen_parse_tree("expr", NULL, 0, 3);
                            AST_NODE *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            gen_ast(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | term        { gen_parse_tree("expr", NULL, 0, 1); } ;
term      :   term
              BIARITHOP2  { gen_parse_tree("BIARITHOP", yytext, 1, 0); gen_ast(yytext, ARITHOPBLOCK, 0, 0); }
              factor      { gen_parse_tree("term", NULL, 0, 3);
                            AST_NODE *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            gen_ast(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | factor      { gen_parse_tree("term", NULL, 0, 1); } ;
factor    :   POPEN       { gen_parse_tree("POPEN", "(", 1, 0); }
              expr
              PCLOSE      { gen_parse_tree("PCLOSE", ")", 1, 0); gen_parse_tree("factor", NULL, 0, 3); }
            | UNIARITHOP  { gen_parse_tree("UNIARITHOP", yytext, 1, 0); gen_ast(yytext, ARITHOPBLOCK, 0, 0); }
              factor      { gen_parse_tree("factor", NULL, 0, 2);
                            AST_NODE *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            gen_ast(tmp->token, tmp->type, 1, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | ID          { store_symbols(yytext);
                            gen_parse_tree("ID", yytext, 1, 0);
                            gen_parse_tree("factor", NULL, 0, 1);
                            gen_ast(yytext, IDBLOCK, 0, 0);
                          }
            | NUM         { gen_parse_tree("NUM", yytext, 1, 0);
                            gen_parse_tree("factor", NULL, 0, 1);
                            gen_ast(yytext, NUMBLOCK, 0, 0);
                          } ;
%%

int main(int argc, char **argv) {
  symbol_table = (SYMBOL_TABLE_NODE *) calloc(1, sizeof(SYMBOL_TABLE_NODE));
  parse_stack = (PARSE_TREE_NODE **) calloc(STACK_SIZE, sizeof(PARSE_TREE_NODE *));
  ast_stack = (AST_NODE **) calloc(STACK_SIZE, sizeof(AST_NODE *));
  max_line_number = parse_stack_top = parse_tree_node_id = ast_stack_top = ast_node_id = 0;
  yyin = fopen(argv[1], "r");
  yyparse();
  parse_tree = parse_stack[0];
  ast = ast_stack[0];
  meet_node_visualization_id = ast_node_id;
  cfg_node_bucket = (CFG_NODE **) calloc(max_line_number + 1, sizeof(CFG_NODE *));
  cfg = (CFG_NODE *) calloc(1, sizeof(CFG_NODE));
  gen_cfg(cfg, ast);
  free(cfg_node_bucket);

  //pretty_print_parse_tree(parse_tree);
  generate_dot_file("parse-tree.dot", "Parse_Tree", PARSE_TREE);
  generate_dot_file("ast.dot", "Abstract_Syntax_Tree", AST);
  generate_dot_file("cfg.dot", "Control_Flow_Graph", CFG);
  return 0;
}