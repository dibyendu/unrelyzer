%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "grammar.tab.h"

size_t line_number = 0;
bool is_new_stmt = false;

%}

%option noyywrap
%s SIGNED_NUMBER_STATE
%s SINGLE_LINE_COMMENT

INTEGER								([1-9]+[0-9]*|0)
IDENTIFIER							[_a-zA-Z][_a-zA-Z0-9]*
%%
\/\/								{ BEGIN(SINGLE_LINE_COMMENT); }
<SINGLE_LINE_COMMENT>.*				{ BEGIN(INITIAL); }
\n									{ ++line_number; }
[ \t]								;
;									{ is_new_stmt = true; return SEMICOLON; }
,									{ return COMMA; }
"void"								{ BEGIN(INITIAL); return VOID; }
"int"								{ BEGIN(INITIAL); return INT; }
"if"								{ is_new_stmt = false; BEGIN(INITIAL); return IF; }
"else"								{ BEGIN(INITIAL); return ELSE; }
"while"								{ is_new_stmt = false; BEGIN(INITIAL); return WHILE; }
"return"							{ is_new_stmt = false; BEGIN(INITIAL); return RETURN; }
{IDENTIFIER}						{ BEGIN(INITIAL);
									  if (yylval) free(yylval);
									  yylval = (char *) malloc(strlen(yytext) + 1);
									  strcpy(yylval, yytext);
									  return ID;
									}
{INTEGER}							{ BEGIN(INITIAL);
									  if (yylval) free(yylval);
									  yylval = (char *) malloc(strlen(yytext) + 1);
									  strcpy(yylval, yytext);
									  return NUM;
									}
<SIGNED_NUMBER_STATE>[+\-]			{ BEGIN(INITIAL); return UNIARITHOP; }
[+\-]								{ BEGIN(SIGNED_NUMBER_STATE); return ADDOP; }
[*%/]								{ BEGIN(SIGNED_NUMBER_STATE); return MULOP; }
!									{ BEGIN(SIGNED_NUMBER_STATE); return UNILOGOP; }
&&									{ BEGIN(SIGNED_NUMBER_STATE); return AND; }
\|\|								{ BEGIN(SIGNED_NUMBER_STATE); return OR; }
(!=|==)								{ BEGIN(SIGNED_NUMBER_STATE); return LOGEQOP; }
(<|>|<=|>=)							{ BEGIN(SIGNED_NUMBER_STATE); return LOGCMPOP; }
=									{ is_new_stmt = false; BEGIN(SIGNED_NUMBER_STATE); return ASSIGN; }
"("									{ BEGIN(SIGNED_NUMBER_STATE); return POPEN; }
")"									{ BEGIN(INITIAL); return PCLOSE; }
"{"									{ is_new_stmt = true; return BOPEN; }
"}"									{ return BCLOSE; }
.									{ fprintf(stderr, "found unrecognized character ‘%s’ at line #%zu\n", yytext, line_number);
									  exit(EXIT_FAILURE);
									}
%%