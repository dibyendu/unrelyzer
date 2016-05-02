#include "parser.h"

const char *printable_tokens[] = {
  "ID",
  "NUM",
  "UNILOGOP",
  "UNIARITHOP",
  "LOGEQOP",
  "LOGCMPOP",
  "ADDOP",
  "MULOP"
};

void yyerror(const char *s) {
  fprintf(stderr,"%s at line #%zu\n", s, line_number + 1);
}

size_t number_hash(unsigned long number, size_t size) {
  size_t sum = 0;
  while (number) {
    size_t digit = number % 10;
    sum += digit * digit;
    number /= 10;
  }
  return sum % size;
}

size_t string_hash(const char *str, size_t size) {
  unsigned long c, base = 1000;
  size_t hash = 8642;
  while (c = *str++) hash = (hash << 5) + hash + (size_t) c;
  c = 1;
  while (c <= hash) c *= base;
  c /= base;
  return ((size_t) (hash / c) * (size_t) (hash % base)) % size;
}

IntegerSet *insert_into_constant_set(IntegerSet *set, long value) {
  bool key_exists = false;
  return avl_tree_insert(set, value, &key_exists);
}

int parse(const char *file) {
  int ret_val, i, j;
  yyin = fopen(file, "r");
  symbol_table = (SymbolTable *) calloc(SYMBOL_TABLE_SIZE, sizeof(SymbolTable));
  function_table = (FunctionTable *) calloc(FUNCTION_TABLE_SIZE, sizeof(FunctionTable));
  stmt_line_map = (size_t *) calloc(STATEMENT_LINE_BUCKET_SIZE, sizeof(size_t));
  constant_set = NULL;
  parse_stack = (ParseTree **) calloc(PARSE_STACK_SIZE, sizeof(ParseTree *));
  ast_stack = (Ast **) calloc(PARSE_STACK_SIZE, sizeof(Ast *));
  N_variables = N_lines = N_stmts = parse_stack_top = ast_stack_top = 0;
  ret_val = yyparse();
  if (!ret_val) {
    symbol_table_indices = (size_t *) calloc(N_variables, sizeof(size_t));
    for (i = 0, j = 0; i < SYMBOL_TABLE_SIZE; ++i)
      if (symbol_table[i].id)
        symbol_table_indices[j++] = i;
  }
  if (yylval) free(yylval);
  free(parse_stack);
  free(ast_stack);
  fclose(yyin);
  return ret_val;
}

size_t symbol_table_entry(const char *token) {
  size_t index = string_hash(token, SYMBOL_TABLE_SIZE), i;
  while (symbol_table[index].id) {
    if (!strcmp(symbol_table[index].id, token))
      return index;
    index = (index + 1) % SYMBOL_TABLE_SIZE;
  }
  symbol_table[index].id = (char *) calloc(strlen(token) + 1, sizeof(char));
  symbol_table[index].decl_scope = (int *) calloc(FUNCTION_TABLE_SIZE + 1, sizeof(int));
  symbol_table[index].init_scope = (int *) calloc(FUNCTION_TABLE_SIZE + 1, sizeof(int));
  symbol_table[index].use_scope = (int *) calloc(FUNCTION_TABLE_SIZE + 1, sizeof(int));
  for (i = 0; i <= FUNCTION_TABLE_SIZE; i++)
    symbol_table[index].decl_scope[i] = symbol_table[index].init_scope[i] = symbol_table[index].use_scope[i] = -1;
  strcpy(symbol_table[index].id, token);
  N_variables += 1;
  return index;
}

size_t function_table_entry(const char *token) {
  size_t index = string_hash(token, FUNCTION_TABLE_SIZE);
  while (function_table[index].id) {
    if (!strcmp(function_table[index].id, token))
      return index;
    index = (index + 1) % FUNCTION_TABLE_SIZE;
  }
  function_table[index].id = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(function_table[index].id, token);
  return index;
}

void free_constant_set(IntegerSet *set) {
  if (!set) return;
  free_constant_set(set->left);
  free_constant_set(set->right);
  free(set);
}

void free_parse_tree(ParseTree *head) {
  void free_node(ParseTree *node) {
    free(node->token);
    free(node->printable);
    free(node);
  }

  if (!head->number_of_children) {
    free_node(head);
    return;
  }
  int n = head->number_of_children;
  while (n) {
    free_parse_tree(head->children[head->number_of_children - n]);
    n -= 1;
  }
  free_node(head);
}

void free_symbol_table() {
  int i;
  for (i = 0; i < N_variables; ++i) {
    free(symbol_table[symbol_table_indices[i]].id);
    free(symbol_table[symbol_table_indices[i]].decl_scope);
    free(symbol_table[symbol_table_indices[i]].init_scope);
    free(symbol_table[symbol_table_indices[i]].use_scope);
    free(symbol_table[symbol_table_indices[i]].concrete);
    free(symbol_table[symbol_table_indices[i]].abstract);
  }
  free(symbol_table);
  free(symbol_table_indices);
}

void free_function_table() {
  int i;
  for (i = 0; i < FUNCTION_TABLE_SIZE; ++i)
    if (function_table[i].id)
      free(function_table[i].id);
  free(function_table);
}

void free_ast(Ast *node) {
  if (!node) return;
  int i = node->number_of_children;
  while (i) free_ast(node->children[--i]);
  if (node->token[0]) free(node->token);
  if (node->children) free(node->children);
  if (node->value) free(node->value);
  free(node);
}

/*
 * A tree like structure formed in a bottom up manner
 * where the root node contains the start symbol.
 * All the terminals are stored at the bottom
 * level of the parse tree.
 */
void build_parse_tree(const char *token, const char *printable, bool is_terminal, int number_of_children) {
  ParseTree *tmp = (ParseTree *) calloc(1, sizeof(ParseTree));
  tmp->token = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(tmp->token, token);
  if (is_terminal) {
    if (printable) {
      tmp->printable = (char *) calloc(strlen(printable) + 1, sizeof(char));
      strcpy(tmp->printable, printable);
    }
    tmp->is_terminal = true;
    parse_stack[parse_stack_top++] = tmp;
    parse_tree = parse_stack[0];
    return;
  }
  tmp->number_of_children = number_of_children;
  tmp->children = (ParseTree **) calloc(number_of_children, sizeof(ParseTree *));
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = parse_stack[--parse_stack_top];
  parse_stack[parse_stack_top++] = tmp;
  parse_tree = parse_stack[0];
  return;
}

/*
 * Similar to parse tree but with
 * all the redundant nodes pruned out.
 */
void build_abstract_syntax_tree(const char *token, AstType type, OperatorType op, size_t number_of_children, size_t line, size_t stmt) {
  Ast *tmp = (Ast *) calloc(1, sizeof(Ast));
  tmp->token = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(tmp->token, token);
  tmp->number_of_children = number_of_children;
  tmp->type = type;
  tmp->operator_type = op;
  tmp->children = (Ast **) calloc(number_of_children, sizeof(Ast *));
  tmp->line_number = line;
  tmp->stmt_number = stmt;
  N_lines = line > N_lines ? line : N_lines;
  N_stmts = stmt > N_stmts ? stmt : N_stmts;
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = ast_stack[--ast_stack_top];
  ast_stack[ast_stack_top++] = tmp;
  ast = ast_stack[0];
  return;
}

void traverse_parse_tree(void *head, FILE *file) {
  void _traverse_parse_tree(ParseTree *head) {
    if (head->is_terminal) {
      int i = sizeof(printable_tokens)/sizeof(char *);
      while (i--) {
        if (!strcmp(head->token, printable_tokens[i])) {
          fprintf(file, "\t%ld [label=\"%s | %s\", shape=\"record\", fillcolor=\"#00FF00\"];\n",
            (unsigned long) head, head->token, head->printable);
          break;
        }
      }
      if (i < 0)
        fprintf(file, "\t%ld [label=\"%s\", fillcolor=\"#00FF00\", shape=\"box\"];\n", (unsigned long) head, head->token);
    }
    else
      fprintf(file, "\t%ld [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", (unsigned long) head, head->token);
    int i = 0;
    while (i < head->number_of_children) {
      _traverse_parse_tree(head->children[i]);
      fprintf(file, "\t%ld -> %ld;\n", (unsigned long) head, (unsigned long) head->children[i++]);
    }
    return;
  }
  _traverse_parse_tree((ParseTree *) head);
}

void traverse_ast(void *head, FILE *file) {
  void _traverse_ast(Ast *head) {
    char label[100];
    if (head->stmt_number > 0 && head->line_number > 0)
      sprintf(label, "<<sup>%zu</sup>%s<sub>%zu</sub>>", head->stmt_number, head->token, head->line_number);
    else
      sprintf(label, "\"%s\"", head->token);
    fprintf(file, "\t%ld [label=%s, fillcolor=\"#%s\", shape=\"%s\"];\n",
      (unsigned long) head,
      label,
      head->type & TRUEBLOCK ? "00FF00" : head->type & FALSEBLOCK ? "FF0000" : head->type & LOGOPBLOCK ? "FA8072" :
        head->type & IDBLOCK ? "FFFF00" : (head->type & NUMBLOCK || head->type & INTERVALBLOCK) ? "00FFFF" : "FFEFD5",
      head->type & FUNCBLOCK ? "box3d" : head->type & ASSIGNBLOCK ? "larrow" : head->type & CALLBLOCK ? "doublecircle" :
        head->type & CONTROLBLOCK ? "diamond" : (head->type & TRUEBLOCK || head->type & FALSEBLOCK) ? "box" :
        head->type & RETURNBLOCK ? "house" : head->type & LOGOPBLOCK ? "oval" : (head->type & IDBLOCK || head->type & NUMBLOCK ||
        head->type & INTERVALBLOCK) ?
        "plaintext" : "circle");
    size_t i = 0;
    while (i < head->number_of_children) {
      _traverse_ast(head->children[i]);
      fprintf(file, "\t%ld -> %ld;\n", (unsigned long) head, (unsigned long) head->children[i++]);
    }
    return;
  }
  _traverse_ast((Ast *) head);
}

char *print_subsscript(size_t num, char *str) {
  static char *subscript[] = {"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"};
  size_t reverse = 0;
  int digits = 0;
  str[0] = 0;
  while (num) {
    reverse = reverse * 10 + num % 10;
    num /= 10;
    digits += 1;
  }
  digits = digits ? digits : 1;
  while (reverse) {
    strcat(str, subscript[reverse % 10]);
    reverse /= 10;
    digits -= 1;
  }
  while (digits) {
    strcat(str, subscript[0]);
    digits -= 1;
  }
  return str;
}