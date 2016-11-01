%{

#include "parser.h"

bool unary_minus_stack[MAX_UNARY_ARITH_OPERATOR_NESTING] = {false};
size_t unary_minus_stack_top = 0,
	   stmt_number = 0,
	   param_decl_stmt_number,
	   N_params = 0,
	   calling_function_stack[MAX_FUNCTION_CALL_NESTING * 3] = {0},
	   calling_function_stack_top = 0,
	   calling_function_N_args_stack[MAX_FUNCTION_CALL_NESTING + 1] = {0},
	   calling_function_N_args_stack_top = 0;
int current_function_table_index = -1;	// -1 for global scope

void semantic_error(const char *token, ErrorT type) {
  char error[200];
  switch (type) {
    case MULT_DECL_VAR:
      sprintf(error, "multiple declaration of variable ‘%s’", token);
      break;
    case MULT_DECL_PARAM:
      sprintf(error, "multiple declaration of parameter ‘%s’", token);
      break;
    case UN_DECL:
      sprintf(error, "undeclared variable ‘%s’", token);
      break;
    case NO_INIT:
      sprintf(error, "uninitialized variable ‘%s’", token);
      break;
    case UN_DEF:
      sprintf(error, "undefined function ‘%s’, called", token);
      break;
    case MULT_DEF_FUNC:
      sprintf(error, "multiple definition of function ‘%s’", token);
      break;
    case PARAM_MISMATCH:
      sprintf(error, "number of parameters mismatched for function ‘%s’, called", token);
      break;
  }
  yyerror(error);
}

bool is_declared(size_t s_index, size_t f_index, size_t stmt_number) {
  return
	(symbol_table[s_index].decl_scope[f_index] != -1 &&
	symbol_table[s_index].decl_scope[f_index] < stmt_number) ||
	(symbol_table[s_index].decl_scope[FUNCTION_TABLE_SIZE] != -1 &&
	symbol_table[s_index].decl_scope[FUNCTION_TABLE_SIZE] < stmt_number);
}

bool is_initialized(size_t s_index, size_t f_index, size_t stmt_number) {
  return
	symbol_table[s_index].decl_scope[f_index] != -1 ?
	(symbol_table[s_index].init_scope[f_index] != -1 &&
	symbol_table[s_index].init_scope[f_index] < stmt_number) :
	(symbol_table[s_index].init_scope[FUNCTION_TABLE_SIZE] != -1 &&
	symbol_table[s_index].init_scope[FUNCTION_TABLE_SIZE] < stmt_number) ||
	(symbol_table[s_index].init_scope[f_index] != -1 &&
	symbol_table[s_index].init_scope[f_index] < stmt_number);
}

int is_defined_function(const char *func) {
  size_t index = string_hash(func, FUNCTION_TABLE_SIZE);
  while (function_table[index].id) {
    if (!strcmp(function_table[index].id, func))
      return index;
    index = (index + 1) % FUNCTION_TABLE_SIZE;
  }
  return -1;
}

%}

%define api.value.type {char *}

%start program

%token SEMICOLON
%token COMMA
%token POPEN
%token PCLOSE
%token BOPEN
%token BCLOSE
%token VOID
%token INT
%token IF
%token ELSE
%token WHILE
%token RETURN
%token ASSIGN
%token NUM
%token ID
%token OR
%token AND
%token UNIARITHOP
%token UNILOGOP
%token ADDOP
%token MULOP
%token LOGEQOP
%token LOGCMPOP

/*
 * Declaration of the precedence and associativity of operators.
 * The higher the line number the higher the precedence.
 */

%right ASSIGN
%left OR
%left AND
%left LOGEQOP
%left LOGCMPOP
%left ADDOP
%left MULOP
%precedence UNARY

%%
program				:		decl_list			{ build_parse_tree("program", NULL, false, 1);
												  build_abstract_syntax_tree("program", 0, 0, ast_stack_top, 0, 0);
												};

decl_list			:		decl_list
							decl				{ build_parse_tree("decl_list", NULL, false, 2); }
					|		decl				{ build_parse_tree("decl_list", NULL, false, 1); };

decl				:		var_decl			{ build_parse_tree("decl", NULL, false, 1); }
					|		func_decl			{ build_parse_tree("decl", NULL, false, 1); };

var_decl			:		var_type
							id_list
							SEMICOLON			{ build_parse_tree(";", NULL, true, 0);
												  build_parse_tree("var_decl", NULL, false, 3);
												};

id_list				:		id_list
							COMMA				{ build_parse_tree(",", NULL, true, 0); }
							decl_or_def			{ build_parse_tree("id_list", NULL, false, 3); }
					|		decl_or_def			{ build_parse_tree("id_list", NULL, false, 1); };

decl_or_def			:		ID					{ size_t s_index = symbol_table_entry(yylval),
														 f_index = current_function_table_index < 0 ? FUNCTION_TABLE_SIZE : current_function_table_index;
												  stmt_line_map[++stmt_number] = line_number + 1;
												  if (symbol_table[s_index].decl_scope[f_index] == -1)
												  	symbol_table[s_index].decl_scope[f_index] = stmt_number;
												  else {
												  	semantic_error(yylval, MULT_DECL_VAR);
												  	YYABORT;
												  }
												  if (symbol_table[s_index].init_scope[f_index] == -1)
												  	symbol_table[s_index].init_scope[f_index] = stmt_number;
												  build_parse_tree("ID", yylval, true, 0);
												  build_abstract_syntax_tree(yylval, IDBLOCK, 0, 0, 0, 0);
												}
							ASSIGN				{ build_parse_tree("=", NULL, true, 0); }
							expr				{ build_parse_tree("decl_def", NULL, false, 3);
												  build_abstract_syntax_tree("=", ASSIGNBLOCK, 0, 2, line_number + 1, stmt_number);
												}
					|		ID					{ size_t s_index = symbol_table_entry(yylval),
														 f_index = current_function_table_index < 0 ? FUNCTION_TABLE_SIZE : current_function_table_index;
												  stmt_line_map[++stmt_number] = line_number + 1;
												  if (symbol_table[s_index].decl_scope[f_index] == -1)
												  	symbol_table[s_index].decl_scope[f_index] = stmt_number;
												  else {
												  	semantic_error(yylval, MULT_DECL_VAR);
												  	YYABORT;
												  }
												  build_parse_tree("ID", yylval, true, 0);
												  build_parse_tree("decl_def", NULL, false, 1);
												};

func_decl			:		VOID				{ build_parse_tree("void", NULL, true, 0); }
							func_body			{ build_parse_tree("func_decl", NULL, false, 2); }
					|		var_type
							func_body			{ build_parse_tree("func_decl", NULL, false, 2); };

func_body			:		ID					{ current_function_table_index = is_defined_function(yylval);
												  if (current_function_table_index != -1) {
												  	semantic_error(yylval, MULT_DEF_FUNC);
												  	YYABORT;
												  }
												  current_function_table_index = function_table_entry(yylval);
												  build_parse_tree("ID", yylval, true, 0);
												  stmt_line_map[++stmt_number] = line_number + 1;
												  build_abstract_syntax_tree(yylval, FUNCBLOCK, 0, 0, line_number + 1, stmt_number);
												  param_decl_stmt_number = stmt_number;
												}
							POPEN				{ build_parse_tree("(", NULL, true, 0); }
							params				{ size_t stmt_count = ast_stack_top - 1;
												  while (!(ast_stack[stmt_count--]->type & FUNCBLOCK));
												  build_abstract_syntax_tree("param", 0, 0, ast_stack_top - stmt_count - 2, 0, 0);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[ast_stack_top - 1];
												  ast_stack[ast_stack_top - 1] = tmp;
												  function_table[current_function_table_index].N_params = N_params;
												  N_params = 0;
												}
							PCLOSE				{ build_parse_tree(")", NULL, true, 0); }
							compound_stmt		{ current_function_table_index = -1;
												  build_parse_tree("func_body", NULL, false, 5);
												  size_t stmt_count = ast_stack_top - 1;
												  while (!(ast_stack[stmt_count--]->type & FUNCBLOCK));
												  build_abstract_syntax_tree("body", 0, 0, ast_stack_top - stmt_count - 2, 0, 0);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2,
												  	tmp->line_number, tmp->stmt_number);
												  free(tmp->token);
												  free(tmp);
												};

params				:		param_list			{ build_parse_tree("params", NULL, false, 1); }
					|		VOID				{ build_parse_tree("void", NULL, true, 0);
												  build_parse_tree("params", NULL, false, 1);
												}
					|		/* empty */			{ build_parse_tree("ε", NULL, true, 0);
												  build_parse_tree("params", NULL, false, 1);
												};

param_list			:		param_list
							COMMA				{ build_parse_tree(",", NULL, true, 0); }
							param				{ build_parse_tree("param_list", NULL, false, 3); }
					|		param 				{ build_parse_tree("param_list", NULL, false, 1); };

param				:		var_type
							ID					{ size_t s_index = symbol_table_entry(yylval);
												  if (symbol_table[s_index].decl_scope[current_function_table_index] == -1)
												  	symbol_table[s_index].decl_scope[current_function_table_index] =
												  	symbol_table[s_index].init_scope[current_function_table_index] = param_decl_stmt_number;
												  else {
												  	semantic_error(yylval, MULT_DECL_PARAM);
												  	YYABORT;
												  }
												  N_params++;
												  build_parse_tree("ID", yylval, true, 0);
												  build_parse_tree("param", NULL, false, 2);
												  stmt_line_map[++stmt_number] = line_number + 1;
												  build_abstract_syntax_tree(yylval, IDBLOCK, 0, 0, line_number + 1, stmt_number);
												};

var_type			:		INT					{ build_parse_tree("int", NULL, true, 0);
												  build_parse_tree("type", NULL, false, 1);
												};

compound_stmt		:		BOPEN				{ build_parse_tree("{", NULL, true, 0); }
							local_decl_list
							stmt_list
							BCLOSE				{ build_parse_tree("}", NULL, true, 0);
												  build_parse_tree("comp_stmt", NULL, false, 4);
												};

local_decl_list		:		local_decl_list
							var_decl			{ build_parse_tree("local_decl", NULL, false, 2); }
					|		/* empty */			{ build_parse_tree("ε", NULL, true, 0);
												  build_parse_tree("local_decl", NULL, false, 1);
												};

stmt_list			:		stmt_list
							stmt				{ build_parse_tree("stmt_list", NULL, false, 2); }
					|		/* empty */			{ build_parse_tree("ε", NULL, true, 0);
												  build_parse_tree("stmt_list", NULL, false, 1);
												};

stmt				:		expr_stmt			{ build_parse_tree("stmt", NULL, false, 1); }
					|		compound_stmt		{ build_parse_tree("stmt", NULL, false, 1); }
					|		if_stmt				{ build_parse_tree("stmt", NULL, false, 1); }
					|		while_stmt			{ build_parse_tree("stmt", NULL, false, 1); }
					|		return_stmt			{ build_parse_tree("stmt", NULL, false, 1); };

expr_stmt			:		expr
							SEMICOLON			{ build_parse_tree(";", NULL, true, 0);
												  build_parse_tree("expr_stmt", NULL, false, 2);
												  stmt_line_map[++stmt_number] = line_number + 1;
												}
					|		SEMICOLON			{ build_parse_tree(";", NULL, true, 0);
												  build_parse_tree("expr_stmt", NULL, false, 1);
												};

while_stmt			:		WHILE				{ build_parse_tree("while", NULL, true, 0);
												  stmt_line_map[++stmt_number] = line_number + 1;
												}
							POPEN				{ build_parse_tree("(", NULL, true, 0); }
							expr				{ ast_stack[ast_stack_top - 1]->type |= CONDITION;
												  ast_stack[ast_stack_top - 1]->line_number = line_number + 1;
												  ast_stack[ast_stack_top - 1]->stmt_number = stmt_number;
												}
							PCLOSE				{ build_parse_tree(")", NULL, true, 0); }
							stmt				{ build_parse_tree("while_stmt", NULL, false, 5);
												  size_t stmt_count = ast_stack_top - 1;
												  while (!(ast_stack[stmt_count--]->type & CONDITION));
												  build_abstract_syntax_tree("true\nblock", TRUEBLOCK, 0, ast_stack_top - stmt_count - 2, 0, 0);
												  int stmt_line_number = ast_stack[ast_stack_top - 2]->line_number,
												  	  stmt_stmt_number = ast_stack[ast_stack_top - 2]->stmt_number;
												  ast_stack[ast_stack_top - 2]->line_number = ast_stack[ast_stack_top - 2]->stmt_number = 0;
												  ast_stack[ast_stack_top - 2]->type |= LOGOPBLOCK;
												  build_abstract_syntax_tree("while", CONTROLBLOCK, 0, 2, stmt_line_number, stmt_stmt_number);
												};

if_stmt				:		IF					{ build_parse_tree("if", NULL, true, 0);
												  stmt_line_map[++stmt_number] = line_number + 1;
												}
							POPEN				{ build_parse_tree("(", NULL, true, 0); }
							expr				{ ast_stack[ast_stack_top - 1]->type |= CONDITION;
												  ast_stack[ast_stack_top - 1]->line_number = line_number + 1;
												  ast_stack[ast_stack_top - 1]->stmt_number = stmt_number;
												}
							PCLOSE				{ build_parse_tree(")", NULL, true, 0); }
							stmt				{ size_t stmt_count = ast_stack_top - 1;
												  while (!(ast_stack[stmt_count--]->type & CONDITION));
												  build_abstract_syntax_tree("true\nblock", TRUEBLOCK, 0, ast_stack_top - stmt_count - 2, 0, 0);
												}
							else_stmt			{ build_parse_tree("if_stmt", NULL, false, 6);
												  size_t cond_block_offset = ast_stack[ast_stack_top - 1]->type & TRUEBLOCK ? 2 : 3,
												  		 stmt_line_number = ast_stack[ast_stack_top - cond_block_offset]->line_number,
												  		 stmt_stmt_number = ast_stack[ast_stack_top - cond_block_offset]->stmt_number;
												  ast_stack[ast_stack_top - cond_block_offset]->line_number = 0;
												  ast_stack[ast_stack_top - cond_block_offset]->stmt_number = 0;
												  ast_stack[ast_stack_top - cond_block_offset]->type |= LOGOPBLOCK;
												  build_abstract_syntax_tree("if", CONTROLBLOCK, 0, cond_block_offset, stmt_line_number, stmt_stmt_number);
												};

else_stmt			:		/* empty */			{ build_parse_tree("ε", NULL, true, 0);
												  build_parse_tree("else_stmt", NULL, false, 1);
												}
					|		ELSE				{ build_parse_tree("else", NULL, true, 0); }
							stmt				{ build_parse_tree("else_stmt", NULL, false, 2);
												  size_t stmt_count = ast_stack_top - 1;
												  while (!(ast_stack[stmt_count--]->type & TRUEBLOCK));
												  build_abstract_syntax_tree("false\nblock", FALSEBLOCK, 0, ast_stack_top - stmt_count - 2, 0, 0);
												};

return_stmt			:		RETURN				{ build_parse_tree("return", NULL, true, 0); }
							SEMICOLON			{ build_parse_tree(";", NULL, true, 0);
												  build_parse_tree("return_stmt", NULL, false, 2);
												  stmt_line_map[++stmt_number] = line_number + 1;
												  build_abstract_syntax_tree("return", RETURNBLOCK, 0, 0, line_number + 1, stmt_number);
												};
					|		RETURN				{ build_parse_tree("return", NULL, true, 0); }
							expr
							SEMICOLON			{ build_parse_tree(";", NULL, true, 0);
												  build_parse_tree("return_stmt", NULL, false, 3);
												  stmt_line_map[++stmt_number] = line_number + 1;
												  build_abstract_syntax_tree("return", RETURNBLOCK, 0, 1, line_number + 1, stmt_number);
												};

expr				:		ID					{ build_parse_tree("ID", yylval, true, 0);
												  build_abstract_syntax_tree(yylval, IDBLOCK, 0, 0, 0, 0);
												  size_t s_index = symbol_table_entry(yylval);
												  stmt_line_map[++stmt_number] = line_number + 1;
												  if (!is_declared(s_index, current_function_table_index, stmt_number)) {
												  	semantic_error(yylval, UN_DECL);
												  	YYABORT;
												  }
												  if (symbol_table[s_index].init_scope[current_function_table_index] == -1)
												  	symbol_table[s_index].init_scope[current_function_table_index] = stmt_number;
												}
							ASSIGN				{ build_parse_tree("=", NULL, true, 0); }
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  build_abstract_syntax_tree("=", ASSIGNBLOCK, 0, 2, line_number + 1, stmt_number);
												  --stmt_number;
												}
					|		expr
							OR					{ build_parse_tree("\\|\\|", NULL, true, 0);
												  build_abstract_syntax_tree("||", LOGOPBLOCK, LOR, 0, 0, 0);
												}
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		expr
							AND					{ build_parse_tree("&&", NULL, true, 0);
												  build_abstract_syntax_tree("&&", LOGOPBLOCK, LAND, 0, 0, 0);
												}
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		expr
							LOGEQOP				{ build_parse_tree("LOGEQOP", yytext, true, 0);
												  build_abstract_syntax_tree(yytext, LOGOPBLOCK, !strcmp(yytext, "==") ? EQ : NEQ, 0, 0, 0);
												}
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		expr
							LOGCMPOP			{ char printable[10];
												  sprintf(printable, "\\%s", yytext);
												  build_parse_tree("LOGCMPOP", printable, true, 0);
												  build_abstract_syntax_tree(yytext, LOGOPBLOCK, !strcmp(yytext, ">=") ? GEQ :
												  	!strcmp(yytext, ">") ? GT : !strcmp(yytext, "<") ? LT : LEQ, 0, 0, 0);
												}
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		expr
							ADDOP				{ build_parse_tree("ADDOP", yytext, true, 0);
												  build_abstract_syntax_tree(yytext, ARITHOPBLOCK, !strcmp(yytext, "+") ? ADD : SUB, 0, 0, 0);
												}
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		expr
							MULOP				{ build_parse_tree("MULOP", yytext, true, 0);
												  build_abstract_syntax_tree(yytext, ARITHOPBLOCK, !strcmp(yytext, "*") ? MUL :
												  	!strcmp(yytext, "/") ? DIV : REM, 0, 0, 0);
												}
							expr				{ build_parse_tree("expr", NULL, false, 3);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		UNILOGOP			{ build_parse_tree("UNILOGOP", yytext, true, 0);
												  build_abstract_syntax_tree(yytext, LOGOPBLOCK, NOT, 0, 0, 0);
												}
							expr %prec UNARY	{ build_parse_tree("expr", NULL, false, 2);
												  Ast *tmp = ast_stack[ast_stack_top - 2];
												  ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 1, 0, 0);
												  free(tmp->token);
												  free(tmp);
												}
					|		UNIARITHOP			{ build_parse_tree("UNIARITHOP", yytext, true, 0);
												  unary_minus_stack[unary_minus_stack_top++] = !strcmp(yytext, "-") ? true : false;
												  if (unary_minus_stack[unary_minus_stack_top - 1]) {
												  	build_abstract_syntax_tree("0", NUMBLOCK, 0, 0, 0, 0);
												  	build_abstract_syntax_tree(yytext, ARITHOPBLOCK, SUB, 0, 0, 0);
												  }
												}
							expr %prec UNARY	{ build_parse_tree("expr", NULL, false, 2);
												  if (unary_minus_stack[--unary_minus_stack_top]) {
												  	Ast *tmp = ast_stack[ast_stack_top - 2];
												  	ast_stack[ast_stack_top - 2] = ast_stack[--ast_stack_top];
												  	build_abstract_syntax_tree(tmp->token, tmp->type, tmp->operator_type, 2, 0, 0);
												  	free(tmp->token);
												  	free(tmp);
												  }
												}
					|		POPEN				{ build_parse_tree("(", NULL, true, 0); }
							expr
							PCLOSE				{ build_parse_tree(")", NULL, true, 0);
												  build_parse_tree("expr", NULL, false, 3);
												}
					|		ID					{ int f_index = is_defined_function(yylval);
												  if (f_index < 0) {
												  	semantic_error(yylval, UN_DEF);
												  	YYABORT;
												  }
												  build_parse_tree("ID", yylval, true, 0);
												  calling_function_stack[calling_function_stack_top++] = f_index;
												  calling_function_stack[calling_function_stack_top++] = is_new_stmt ? line_number + 1 : 0;
												  calling_function_stack[calling_function_stack_top++] = is_new_stmt ? stmt_number + 1 : 0;
												  calling_function_N_args_stack_top += 1;
												  is_new_stmt = false;
												}
							POPEN				{ build_parse_tree("(", NULL, true, 0); }
							args				{ size_t f_index = calling_function_stack[calling_function_stack_top - 3],
												               line = calling_function_stack[calling_function_stack_top - 2],
												               stmt = calling_function_stack[calling_function_stack_top - 1];
												  calling_function_stack_top -= 3;
												  if (calling_function_N_args_stack[calling_function_N_args_stack_top--] !=
												  	function_table[f_index].N_params) {
												  	semantic_error(function_table[f_index].id, PARAM_MISMATCH);
												  	YYABORT;
												  }
												  calling_function_N_args_stack[calling_function_N_args_stack_top + 1] = 0;
												  build_abstract_syntax_tree(function_table[f_index].id, CALLBLOCK, 0,
												  	function_table[f_index].N_params, line, stmt);
												}
							PCLOSE				{ build_parse_tree(")", NULL, true, 0);
												  build_parse_tree("expr", NULL, false, 4);
												}
					|		ID					{ build_parse_tree("ID", yylval, true, 0);
												  build_parse_tree("expr", NULL, false, 1);
												  build_abstract_syntax_tree(yylval, IDBLOCK, 0, 0, 0, 0);
												  size_t s_index = symbol_table_entry(yylval),
												  		 f_index = current_function_table_index < 0 ? FUNCTION_TABLE_SIZE : current_function_table_index;
												  if (!is_declared(s_index, f_index, stmt_number)) {
												  	semantic_error(yylval, UN_DECL);
												  	YYABORT;
												  }
												  if (!is_initialized(s_index, f_index, stmt_number)) {
												  	semantic_error(yylval, NO_INIT);
												  	YYABORT;
												  }
												  if (symbol_table[s_index].use_scope[f_index] == -1)
												  	symbol_table[s_index].use_scope[f_index] = stmt_number;
												}
					|		NUM					{ build_parse_tree("NUM", yylval, true, 0);
												  build_parse_tree("expr", NULL, false, 1);
												  build_abstract_syntax_tree(yylval, NUMBLOCK, 0, 0, 0, 0);
												  int i = unary_minus_stack_top - 1, is_minus = 0;
												  while (i >= 0) is_minus += unary_minus_stack[i--] ? 1 : 0;
												  constant_set = insert_into_constant_set(constant_set, is_minus % 2 ? - atol(yylval) : atol(yylval));
												};

args				:		arg_list			{ build_parse_tree("args", NULL, false, 1); }
					|		/* empty */			{ build_parse_tree("ε", NULL, true, 0);
												  build_parse_tree("args", NULL, false, 1);
												};

arg_list			:		arg_list
							COMMA				{ build_parse_tree(",", NULL, true, 0); }
							expr				{ build_parse_tree("arg_list", NULL, false, 3);
												  calling_function_N_args_stack[calling_function_N_args_stack_top] += 1;
												}
					|		expr				{ build_parse_tree("arg_list", NULL, false, 1);
												  calling_function_N_args_stack[calling_function_N_args_stack_top] += 1;
												};
%%