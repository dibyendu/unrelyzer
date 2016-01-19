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
program   :   compstmt    { build_parse_tree("program", NULL, 0, 1);
                            build_abstract_syntax_tree("program", 0, ast_stack_top, 0);
                          };
compstmt  :   stmt
              compstmt    { build_parse_tree("compstmt", NULL, 0, 2); }
            | stmt        { build_parse_tree("compstmt", NULL, 0, 1); };
stmt      :   type
              ID          { get_symbol_table_index(yytext);
                            build_parse_tree("ID", yytext, 1, 0);
                          }
              idlist
              SEMICOLON   { build_parse_tree("SEMICOLON", ";", 1, 0);
                            build_parse_tree("stmt", NULL, 0, 4);
                          }
            | ID          { get_symbol_table_index(yytext);
                            build_parse_tree("ID", yytext, 1, 0);
                            build_abstract_syntax_tree(yytext, IDBLOCK, 0, 0);
                          }
              ASSIGN      { build_parse_tree("ASSIGN", "=", 1, 0); }
              expr        { build_abstract_syntax_tree("=", ASSIGNBLOCK, 2, line_number + 1); }
              SEMICOLON   { build_parse_tree("SEMICOLON", ";", 1, 0);
                            build_parse_tree("stmt", NULL, 0, 4);
                          }
            | IF          { build_parse_tree("IF", "if", 1, 0); }
              POPEN       { build_parse_tree("POPEN", "(", 1, 0); }
              condition   { ast_stack[ast_stack_top - 1]->type = CONDITION;
                            ast_stack[ast_stack_top - 1]->line_number = line_number + 1;
                          }
              PCLOSE      { build_parse_tree("PCLOSE", ")", 1, 0); }
              stmtblock   { unsigned int stmt_count = ast_stack_top - 1;
                            while (ast_stack[stmt_count--]->type != CONDITION);
                            build_abstract_syntax_tree("true\nblock", TRUEBLOCK, ast_stack_top - stmt_count - 2, 0);
                          }
              elsestmt    { build_parse_tree("stmt", NULL, 0, 6);
                            int cond_block_offset = ast_stack[ast_stack_top - 1]->type == TRUEBLOCK ? 2 : 3,
                                  stmt_line_number = ast_stack[ast_stack_top - cond_block_offset]->line_number;
                            ast_stack[ast_stack_top - cond_block_offset]->line_number = 0;
                            ast_stack[ast_stack_top - cond_block_offset]->type = LOGOPBLOCK;
                            build_abstract_syntax_tree("if", CONTROLBLOCK, cond_block_offset, stmt_line_number);
                          }
            | WHILE       { build_parse_tree("WHILE", "while", 1, 0); }
              POPEN       { build_parse_tree("POPEN", "(", 1, 0); }
              condition   { ast_stack[ast_stack_top - 1]->type = CONDITION;
                            ast_stack[ast_stack_top - 1]->line_number = line_number + 1;
                          }
              PCLOSE      { build_parse_tree("PCLOSE", ")", 1, 0); }
              stmtblock   { build_parse_tree("stmt", NULL, 0, 5);
                            unsigned int stmt_count = ast_stack_top - 1;
                            while (ast_stack[stmt_count--]->type != CONDITION);
                            build_abstract_syntax_tree("true\nblock", TRUEBLOCK, ast_stack_top - stmt_count - 2, 0);
                            int stmt_line_number = ast_stack[ast_stack_top - 2]->line_number;
                            ast_stack[ast_stack_top - 2]->line_number = 0;
                            ast_stack[ast_stack_top - 2]->type = LOGOPBLOCK;
                            build_abstract_syntax_tree("while", CONTROLBLOCK, 2, stmt_line_number);
                          };
type      :   INT         { build_parse_tree("INT", yytext, 1, 0);
                            build_parse_tree("type", NULL, 0, 1);
                          };
idlist    :   /* empty */ { build_parse_tree("EPSILON", "e", 1, 0);
                            build_parse_tree("idlist", NULL, 0, 1);
                          }
            | COMMA       { build_parse_tree("COMMA", ",", 1, 0); }
              ID          { get_symbol_table_index(yytext);
                            build_parse_tree("ID", yytext, 1, 0);
                          }
              idlist      { build_parse_tree("idlist", NULL, 0, 3); };
elsestmt  :   /* empty */ { build_parse_tree("EPSILON", "e", 1, 0);
                            build_parse_tree("elsestmt", NULL, 0, 1);
                          }
            | ELSE        { build_parse_tree("ELSE", "else", 1, 0); }
              stmtblock   { build_parse_tree("elsestmt", NULL, 0, 2);
                            unsigned int stmt_count = ast_stack_top - 1;
                            while (ast_stack[stmt_count--]->type != TRUEBLOCK);
                            build_abstract_syntax_tree("false\nblock", FALSEBLOCK, ast_stack_top - stmt_count - 2, 0);
                          };
stmtblock :   BOPEN       { build_parse_tree("BOPEN", "{", 1, 0); }
              compstmt
              BCLOSE      { build_parse_tree("BCLOSE", "}", 1, 0);
                            build_parse_tree("stmtblock", NULL, 0, 3);
                          }
            | stmt        { build_parse_tree("stmtblock", NULL, 0, 1); };
condition :   condition
              BILOGOP1    { build_parse_tree("BILOGOP", yytext, 1, 0);
                            build_abstract_syntax_tree(yytext, LOGOPBLOCK, 0, 0);
                          }
              condterm    { build_parse_tree("condition", NULL, 0, 3);
                            Ast *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            build_abstract_syntax_tree(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | condterm    { build_parse_tree("condition", NULL, 0, 1); };
condterm  :   condterm
              BILOGOP2    { build_parse_tree("BILOGOP", yytext, 1, 0);
                            build_abstract_syntax_tree(yytext, LOGOPBLOCK, 0, 0);
                          }
              condfactor  { build_parse_tree("condterm", NULL, 0, 3);
                            Ast *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            build_abstract_syntax_tree(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | condfactor  { build_parse_tree("condterm", NULL, 0, 1); };
condfactor :  expr        { build_parse_tree("condfactor", NULL, 0, 1); }
            | UNILOGOP    { build_parse_tree("UNILOGOP", yytext, 1, 0);
                            build_abstract_syntax_tree(yytext, LOGOPBLOCK, 0, 0);
                          }
              expr        { build_parse_tree("condfactor", NULL, 0, 2);
                            Ast *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            build_abstract_syntax_tree(tmp->token, tmp->type, 1, 0);
                            free(tmp->token);
                            free(tmp);
                          };
expr      :   expr
              BIARITHOP1  { build_parse_tree("BIARITHOP", yytext, 1, 0);
                            build_abstract_syntax_tree(yytext, ARITHOPBLOCK, 0, 0);
                          }
              term        { build_parse_tree("expr", NULL, 0, 3);
                            Ast *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            build_abstract_syntax_tree(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | term        { build_parse_tree("expr", NULL, 0, 1); };
term      :   term
              BIARITHOP2  { build_parse_tree("BIARITHOP", yytext, 1, 0);
                            build_abstract_syntax_tree(yytext, ARITHOPBLOCK, 0, 0);
                          }
              factor      { build_parse_tree("term", NULL, 0, 3);
                            Ast *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            build_abstract_syntax_tree(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | factor      { build_parse_tree("term", NULL, 0, 1); };
factor    :   POPEN       { build_parse_tree("POPEN", "(", 1, 0);
                            is_negative_number = false;
                          }
              expr
              PCLOSE      { build_parse_tree("PCLOSE", ")", 1, 0);
                            build_parse_tree("factor", NULL, 0, 3);
                          }
            | UNIARITHOP  { build_parse_tree("UNIARITHOP", yytext, 1, 0);
                            is_negative_number = true;
                            build_abstract_syntax_tree("0", NUMBLOCK, 0, 0);
                            build_abstract_syntax_tree(yytext, ARITHOPBLOCK, 0, 0);
                          }
              factor      { build_parse_tree("factor", NULL, 0, 2);
                            Ast *tmp = ast_stack[ast_stack_top - 2];
                            ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
                            ast_stack_top -= 1;
                            build_abstract_syntax_tree(tmp->token, tmp->type, 2, 0);
                            free(tmp->token);
                            free(tmp);
                          }
            | ID          { get_symbol_table_index(yytext);
                            is_negative_number = false;
                            build_parse_tree("ID", yytext, 1, 0);
                            build_parse_tree("factor", NULL, 0, 1);
                            build_abstract_syntax_tree(yytext, IDBLOCK, 0, 0);
                          }
            | NUM         { constant_set = insert_into_constant_set(constant_set, is_negative_number ? 0 - atoi(yytext) : atoi(yytext));
                            is_negative_number = false;
                            build_parse_tree("NUM", yytext, 1, 0);
                            build_parse_tree("factor", NULL, 0, 1);
                            build_abstract_syntax_tree(yytext, NUMBLOCK, 0, 0);
                          };
%%