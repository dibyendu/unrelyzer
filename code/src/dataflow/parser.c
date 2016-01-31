#include "parser.h"

const char *printable_tokens[] = {
  "ID",
  "NUM",
  "UNIARITHOP",
  "BIARITHOP",
  "BILOGOP",
  "UNILOGOP",
};

typedef struct VisitedCfgNodeT {
  unsigned long ptr;
  bool is_visited;
} VisitedCfgNode;

VisitedCfgNode *cfg_node_hash_table;

int yywrap() {
  return 1;
}

int yyerror(const char *s) {
  fprintf(stderr,"\"%s\" at line # %d\n", s, line_number + 1);
  return 1;
}

int get_cfg_node_index(unsigned long value) {
  int _hash(unsigned long number) {
    int sum = 0;
    while (number) {
      int digit = number % 10;
      sum += digit * digit;
      number /= 10;
    }
    return sum % MAX_CFG_NODES;
  }

  int index = _hash(value);
  while (cfg_node_hash_table[index].ptr) {
    if (cfg_node_hash_table[index].ptr == value) return index;
    index = (index + 1) % MAX_CFG_NODES;
  }
  cfg_node_hash_table[index].ptr = value;
  return index;
}

IntSet *insert_into_constant_set(IntSet *set, int value) {
  bool key_exists = false;
  return avl_tree_insert(set, value, &key_exists);
}

void parse(const char *file) {
  yyin = fopen(file, "r");
  symbol_table = (SymbolTable *) calloc(SYMBOL_TABLE_SIZE, sizeof(SymbolTable));
  constant_set = NULL;
  parse_stack = (ParseTree **) calloc(STACK_SIZE, sizeof(ParseTree *));
  ast_stack = (Ast **) calloc(STACK_SIZE, sizeof(Ast *));
  N_variables = N_lines = parse_stack_top = ast_stack_top = 0;
  is_negative_number = false;
  yyparse();
  symbol_table_indices = (unsigned short *) calloc(N_variables, sizeof(unsigned short));
  int i, j;
  for (i = 0, j = 0; i < SYMBOL_TABLE_SIZE; ++i)
    if (symbol_table[i].id)
      symbol_table_indices[j++] = i;
  cfg_node_bucket = (Cfg **) calloc(N_lines + 1, sizeof(Cfg *));
  cfg = (Cfg *) calloc(1, sizeof(Cfg));
  cfg_node_hash_table = (VisitedCfgNode *) calloc(MAX_CFG_NODES, sizeof(VisitedCfgNode));
  get_cfg_node_index((unsigned long) cfg);
  free(parse_stack);
  free(ast_stack);
  fclose(yyin);
  return;
}

unsigned int hash(const char *str) {
  unsigned long c, hash = 5381, base = 1000;
  while (c = *str++) hash = ((hash << 5) + hash) + c;
  c = 1;
  while (c <= hash) c *= base;
  c /= base;
  return ((int) (hash / c) * (int) (hash % base)) % SYMBOL_TABLE_SIZE;
}

int get_symbol_table_index(const char *token) {
  int index = hash(token);
  while (symbol_table[index].id) {
    if (!strcmp(symbol_table[index].id, token))
      return index;
    index = (index + 1) % SYMBOL_TABLE_SIZE;
  }
  symbol_table[index].id = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(symbol_table[index].id, token);
  N_variables += 1;
  return index;
}

void free_constant_set(IntSet *set) {
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
    free(symbol_table[symbol_table_indices[i]].concrete);
    free(symbol_table[symbol_table_indices[i]].abstract);
  }
  free(symbol_table);
  free(symbol_table_indices);
  free_constant_set(constant_set);
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
void build_parse_tree(const char *token, const char *printable, int is_terminal, int number_of_children) {
  if (is_terminal) {
    ParseTree *tmp = (ParseTree *) calloc(1, sizeof(ParseTree));
    tmp->token = (char *) calloc(strlen(token) + 1, sizeof(char));
    tmp->printable = (char *) calloc(strlen(printable) + 1, sizeof(char));
    strcpy(tmp->token, token);
    strcpy(tmp->printable, printable);
    tmp->is_terminal = is_terminal;
    parse_stack[parse_stack_top] = tmp;
    parse_stack_top += 1;
    parse_tree = parse_stack[0];
    return;
  }
  ParseTree *tmp = (ParseTree *) calloc(1, sizeof(ParseTree));
  tmp->token = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(tmp->token, token);
  tmp->number_of_children = number_of_children;
  tmp->children = (ParseTree **) calloc(number_of_children, sizeof(ParseTree *));
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = parse_stack[--parse_stack_top];
  parse_stack[parse_stack_top++] = tmp;
  parse_tree = parse_stack[0];
  return;
}

void build_abstract_syntax_tree(const char *token, AstType type, int number_of_children, int line) {
  Ast *tmp = (Ast *) calloc(1, sizeof(Ast));
  tmp->token = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(tmp->token, token);
  tmp->number_of_children = number_of_children;
  tmp->type = type;
  tmp->children = (Ast **) calloc(number_of_children, sizeof(Ast *));
  tmp->line_number = line;
  if (line > N_lines)
    N_lines = line;
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = ast_stack[--ast_stack_top];
  ast_stack[ast_stack_top++] = tmp;
  ast = ast_stack[0];
  return;
}

void build_control_flow_graph(Cfg *cfg, Ast *ast) {
  static Cfg *last_node = NULL;

  void _set_feedback_path(Cfg *cfg, Ast *ast) {
    Cfg *feedback_node;
    if (!strcmp(ast->children[ast->number_of_children - 1]->token, "while")) {
      feedback_node = cfg_node_bucket[ast->children[ast->number_of_children - 1]->line_number];
      feedback_node->out_false = cfg;  
    }
    else if (!strcmp(ast->children[ast->number_of_children - 1]->token, "if")) {
      feedback_node = last_node->in1;
      feedback_node->out_true = cfg;
    }
    else {
      feedback_node = cfg_node_bucket[ast->children[ast->number_of_children - 1]->line_number];
      feedback_node->out_true = cfg;
    }
    cfg->in2 = feedback_node;
    return;
  }

  void _handle_control_node(Ast *node) {
    if (!strcmp(node->token, "while")) {
      cfg->statement = node->children[0];
      cfg->program_point = node->line_number;
      cfg_node_bucket[cfg->program_point] = cfg;
      cfg_node_bucket[0] = last_node = cfg->out_true = (Cfg *) calloc(1, sizeof(Cfg));
      get_cfg_node_index((unsigned long) last_node);
      cfg->out_true->in1 = cfg;
      build_control_flow_graph(cfg->out_true, node->children[1]);
      _set_feedback_path(cfg, node->children[1]);
      last_node->in1 = cfg;
      cfg = cfg->out_false = last_node;
    }
    else {
      cfg->statement = node->children[0];
      cfg->program_point = node->line_number;
      cfg_node_bucket[cfg->program_point] = cfg;
      cfg_node_bucket[0] = last_node = cfg->out_true = (Cfg *) calloc(1, sizeof(Cfg));
      get_cfg_node_index((unsigned long) last_node);
      cfg->out_true->in1 = cfg;
      build_control_flow_graph(cfg->out_true, node->children[1]);
      Cfg *if_true_last_node = last_node;
      if (node->number_of_children > 2) {
        cfg_node_bucket[0] = last_node = cfg->out_false = (Cfg *) calloc(1, sizeof(Cfg));
        get_cfg_node_index((unsigned long) last_node);
        cfg->out_false->in1 = cfg;
        build_control_flow_graph(cfg->out_false, node->children[2]);
        last_node->in1->out_true = if_true_last_node;
        if_true_last_node->in2 = last_node->in1;
        free(last_node);
        last_node = if_true_last_node;
      }
      else {
        cfg->out_false = last_node;
        last_node->in2 = cfg;
      }
      last_node->program_point = MEET_NODE;
      cfg_node_bucket[0] = last_node->out_true = (Cfg *) calloc(1, sizeof(Cfg));
      get_cfg_node_index((unsigned long) last_node->out_true);
      last_node->out_true->in1 = last_node;
      cfg = last_node = last_node->out_true;
    }
    return;
  }

  int i = 0;
  while (i < ast->number_of_children) {
    if (ast->children[i]->type == CONTROLBLOCK) {
      _handle_control_node(ast->children[i]);
      i += 1;
      continue;
    }
    cfg->program_point = ast->children[i]->line_number;
    cfg_node_bucket[cfg->program_point] = cfg;
    cfg->statement = ast->children[i];
    cfg_node_bucket[0] = last_node = cfg->out_true = (Cfg *) calloc(1, sizeof(Cfg));
    get_cfg_node_index((unsigned long) last_node);
    cfg->out_true->in1 = cfg;
    cfg = cfg->out_true;
    i += 1;
  }
  return;
}

void generate_dot_file(const char *file, GraphT type) {
  FILE *dot_file = fopen(file, "wt");

  void traverse_parse_tree(ParseTree *head) {
    if (head->is_terminal) {
      int i = sizeof(printable_tokens)/sizeof(char *);
      while (i--) {
        if (!strcmp(head->token, printable_tokens[i])) {
          fprintf(dot_file, "\t%ld [label=\"%s | %s\", shape=\"record\", fillcolor=\"#FFEFD5\"];\n",
            (unsigned long) head, head->token, head->printable);
          break;
        }
      }
      if (i < 0)
        fprintf(dot_file, "\t%ld [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"box\"];\n", (unsigned long) head, head->token);
    }
    else
      fprintf(dot_file, "\t%ld [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", (unsigned long) head, head->token);
    int i = 0;
    while (i < head->number_of_children) {
      traverse_parse_tree(head->children[i]);
      fprintf(dot_file, "\t%ld -> %ld;\n", (unsigned long) head, (unsigned long) head->children[i++]);
    }
    return;
  }

  void traverse_ast(Ast *head) {
    char xlabel[14];
    sprintf(xlabel, "xlabel=\"%d\"", head->line_number);
    fprintf(dot_file, "\t%ld [label=\"%s\", %s fillcolor=\"#%s\", shape=\"%s\"];\n",
        (unsigned long) head, head->token, head->line_number > 0 ? xlabel : "", head->type == TRUEBLOCK ? "00FF00" :
        (head->type == FALSEBLOCK ? "FF0000" : (head->type == IDBLOCK || head->type == NUMBLOCK) ? "FFFF00" : "FFEFD5"),
        head->type == CONTROLBLOCK ? "diamond" : (head->type == TRUEBLOCK || head->type == FALSEBLOCK) ? "box" :
        ((head->type == IDBLOCK || head->type == NUMBLOCK) ? "plaintext" : head->type == ASSIGNBLOCK ? "larrow" : 
          head->type == LOGOPBLOCK ? "oval" :"circle"));
    int i = 0;
    while (i < head->number_of_children) {
      traverse_ast(head->children[i]);
      fprintf(dot_file, "\t%ld -> %ld;\n", (unsigned long) head, (unsigned long) head->children[i++]);
    }
    return;
  }

  void traverse_cfg(Cfg *head) {
    if (!head || cfg_node_hash_table[get_cfg_node_index((unsigned long) head)].is_visited)
      return;
    cfg_node_hash_table[get_cfg_node_index((unsigned long) head)].is_visited = true;
    char label[1024] = {0}, stmt[1024] = {0};
    sprintf(stmt, "%d", head->program_point ? head->program_point : N_lines + 1);
    sprintf(label, "<V<sub>%s</sub>>", head->program_point == MEET_NODE ? "meet" : stmt);
    fprintf(dot_file, "\t%ld [label=%s,  fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", (unsigned long) head, label);
    traverse_cfg(head->out_true);
    traverse_cfg(head->out_false);
    if (head->out_true) {
      stmt[0] = 0;
      fprintf(dot_file, "\t%ld -> %ld [label=\"%s\"];\n", (unsigned long) head, (unsigned long) head->out_true,
        expression_to_string(head->statement, stmt));
    }
    if (head->out_false) {
      stmt[0] = 0;
      fprintf(dot_file, "\t%ld -> %ld [label=\"%s\"];\n", (unsigned long) head, (unsigned long) head->out_false,
        expression_to_string(invert_expression(head->statement), stmt));
    }
    return;
  }

  fprintf(dot_file, "digraph %s {\n\n\trankdir=TB;\n\tnode [style=\"filled\"];\n\n",
    type == PARSE_TREE ? "Parse_Tree" : (type == AST ? "Abstract_Syntax_Tree" : "Control_Flow_Graph"));
  if (type == PARSE_TREE)
    traverse_parse_tree(parse_tree);
  else if (type == AST)
    traverse_ast(ast);
  else {
    traverse_cfg(cfg);
    free(cfg_node_hash_table);
  }
  fprintf(dot_file, "\n}");
  fclose(dot_file);
  return;
}