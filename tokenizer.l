%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"

FILE *fp;
int line_number = 0, i;
const char *keywords[] = {"int", "if", "else", "while"};
const int keyword_tokens[] = {INT, IF, ELSE, WHILE},
          no_of_keywords = sizeof(keywords)/sizeof(char *);
%}

%s SIGNED_NUMBER_STATE
INTEGER                             ([1-9]+[0-9]*|0)
IDENTIFIER                          [_a-zA-Z][_a-zA-Z0-9]*

%%
\n                                  { ++line_number; }
[ \t]                               ;
;                                   { return SEMICOLON; }
,                                   { return COMMA; }
{INTEGER}                           { BEGIN(INITIAL); return NUM; }
{IDENTIFIER}                        {
                                      BEGIN(INITIAL);
                                      for(i = 0; i < no_of_keywords; i++)
                                        if(!strcmp(keywords[i], yytext))
                                          break;
                                      if (i < no_of_keywords)
                                        return keyword_tokens[i];
                                      else
                                        return ID;
                                    }
<SIGNED_NUMBER_STATE>[+\-]          { BEGIN(INITIAL); return UNIARITHOP; }
[+\-]                               { BEGIN(SIGNED_NUMBER_STATE); return BIARITHOP1; }
[*%/]                               { BEGIN(SIGNED_NUMBER_STATE); return BIARITHOP2; }
!                                   { BEGIN(SIGNED_NUMBER_STATE); return UNILOGOP; }
(!=|==)                             { BEGIN(SIGNED_NUMBER_STATE); return BILOGOP1; }
(<|>|<=|>=)                         { BEGIN(SIGNED_NUMBER_STATE); return BILOGOP2; }
=                                   { BEGIN(SIGNED_NUMBER_STATE); return ASSIGN; }
"("                                 { BEGIN(SIGNED_NUMBER_STATE); return POPEN; }
")"                                 { BEGIN(INITIAL); return PCLOSE; }
"{"                                 { return BOPEN; }
"}"                                 { return BCLOSE; }
.                                   { printf("Found unrecognized character \"%s\" at line # %d\n", yytext, line_number); }
%%