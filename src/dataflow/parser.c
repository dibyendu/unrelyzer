#include "parser.h"

const char *printable_tokens[] = {
  "ID",
  "NUM",
  "UNIARITHOP",
  "BIARITHOP",
  "BILOGOP",
  "UNILOGOP",
};

int yywrap() {
  return 1;
}

int yyerror(const char *s) {
  fprintf(stderr,"\"%s\" at line # %d\n", s, line_number + 1);
  return 1;
}

void parse(const char *file) {
  yyin = fopen(file, "r");
  symbol_table = (SymbolTable *) calloc(SYMBOL_TABLE_SIZE, sizeof(SymbolTable));
  parse_stack = (ParseTree **) calloc(STACK_SIZE, sizeof(ParseTree *));
  ast_stack = (Ast **) calloc(STACK_SIZE, sizeof(Ast *));
  max_line_number = parse_stack_top = parse_tree_node_id = ast_stack_top = ast_node_id = 0;
  yyparse();
  cfg_node_bucket = (Cfg **) calloc(max_line_number + 1, sizeof(Cfg *));
  cfg = (Cfg *) calloc(1, sizeof(Cfg));
  free(parse_stack);
  free(ast_stack);
  fclose(yyin);
  return;
}

unsigned int hash(const char *str) {
  unsigned long c, hash = 5381, base = 1000;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c;
  c = 1;
  while (c <= hash)
    c *= base;
  c /= base;

  return ((int) (hash / c) * (int) (hash % base)) % SYMBOL_TABLE_SIZE;
}

void store_symbols(const char *token) {
  int index = hash(token);
  while (symbol_table[index].id)
    index = (index + 1) % SYMBOL_TABLE_SIZE;
  symbol_table[index].id = (char *) calloc(strlen(token) + 1, sizeof(char));
  strcpy(symbol_table[index].id, token);
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
    tmp->visualization_id = parse_tree_node_id++;
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
  tmp->visualization_id = parse_tree_node_id++;
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
  tmp->visualization_id = ast_node_id++;
  tmp->line_number = line;
  if (line > max_line_number)
    max_line_number = line;
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = ast_stack[--ast_stack_top];
  ast_stack[ast_stack_top++] = tmp;
  meet_node_visualization_id = ast_node_id;
  ast = ast_stack[0];
  return;
}

void build_control_flow_graph(Cfg *cfg, Ast *ast) {
  static Cfg *last_node = NULL;

  void set_feedback_path(Cfg *cfg, Ast *ast) {
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

  void handle_control_node(Ast *node) {
    if (!strcmp(node->token, "while")) {
      cfg->statement = node->children[0];
      cfg->program_point = node->line_number;
      cfg_node_bucket[cfg->program_point] = cfg;
      cfg->visualization_id = node->visualization_id;
      cfg_node_bucket[0] = last_node = cfg->out_true = (Cfg *) calloc(1, sizeof(Cfg));
      cfg->out_true->in1 = cfg;
      build_control_flow_graph(cfg->out_true, node->children[1]);
      set_feedback_path(cfg, node->children[1]);
      last_node->in1 = cfg;
      cfg = cfg->out_false = last_node;
    }
    else {
      cfg->statement = node->children[0];
      cfg->program_point = node->line_number;
      cfg_node_bucket[cfg->program_point] = cfg;
      cfg->visualization_id = node->visualization_id;
      cfg_node_bucket[0] = last_node = cfg->out_true = (Cfg *) calloc(1, sizeof(Cfg));
      cfg->out_true->in1 = cfg;
      build_control_flow_graph(cfg->out_true, node->children[1]);
      Cfg *if_true_last_node = last_node;
      if (node->number_of_children > 2) {
        cfg_node_bucket[0] = last_node = cfg->out_false = (Cfg *) calloc(1, sizeof(Cfg));
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
      last_node->visualization_id = meet_node_visualization_id++;
      cfg_node_bucket[0] = last_node->out_true = (Cfg *) calloc(1, sizeof(Cfg));
      last_node->out_true->in1 = last_node;
      cfg = last_node = last_node->out_true;
    }
    return;
  }

  int i = 0;
  while (i < ast->number_of_children) {
    if (ast->children[i]->type == CONTROLBLOCK) {
      handle_control_node(ast->children[i]);
      i += 1;
      continue;
    }
    cfg->program_point = ast->children[i]->line_number;
    cfg_node_bucket[cfg->program_point] = cfg;
    cfg->visualization_id = ast->children[i]->visualization_id;
    cfg->statement = ast->children[i];
    cfg_node_bucket[0] = last_node = cfg->out_true = (Cfg *) calloc(1, sizeof(Cfg));
    cfg->out_true->in1 = cfg;
    cfg = cfg->out_true;
    i += 1;
  }
  return;
}

/*
 * Simple DFS of the parse tree
 */
void pretty_print_parse_tree(ParseTree *head) {
  static int level = 0, space = 0, leave_space = 0;
  int i;
  if (leave_space) {
    leave_space = 0;
    i = space + (level - 1) * 5;
    while (i--)
      putchar(' ');
    printf(" --> ");
  }
  printf("%s", head->token);
  if (head->is_terminal) {
    leave_space = 1;
    int i = sizeof(printable_tokens)/sizeof(char *);
    while (i--) {
      if (!strcmp(head->token, printable_tokens[i])) {
        printf(" (%s)", head->printable);
        break;
      }
    }
    putchar('\n');
  } else {
    printf(" --> ");
    space += strlen(head->token);
  }
  i = 0;
  while (i < head->number_of_children) {
    level += 1;
    pretty_print_parse_tree(head->children[i++]);
  }
  level -= 1;
  if (!head->is_terminal)
    space -= strlen(head->token);
  return;
}

void generate_dot_file(const char *file, GraphT type) {
  FILE *dot_file = fopen(file, "wt");

  void traverse_parse_tree(ParseTree *head) {
    if (head->is_terminal) {
      int i = sizeof(printable_tokens)/sizeof(char *);
      while (i--) {
        if (!strcmp(head->token, printable_tokens[i])) {
          fprintf(dot_file, "\t%d [label=\"%s | %s\", shape=\"record\", fillcolor=\"#FFEFD5\"];\n",
            head->visualization_id, head->token, head->printable);
          break;
        }
      }
      if (i < 0)
        fprintf(dot_file, "\t%d [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"box\"];\n", head->visualization_id, head->token);
    }
    else
      fprintf(dot_file, "\t%d [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", head->visualization_id, head->token);
    int i = 0;
    while (i < head->number_of_children) {
      traverse_parse_tree(head->children[i]);
      fprintf(dot_file, "\t%d -> %d;\n", head->visualization_id, head->children[i++]->visualization_id);
    }
    return;
  }

  void traverse_ast(Ast *head) {
    char xlabel[14];
    sprintf(xlabel, "xlabel=\"%d\"", head->line_number);
    fprintf(dot_file, "\t%d [label=\"%s\", %s fillcolor=\"#%s\", shape=\"%s\"];\n",
        head->visualization_id, head->token, head->line_number > 0 ? xlabel : "", head->type == TRUEBLOCK ? "00FF00" :
        (head->type == FALSEBLOCK ? "FF0000" : (head->type == IDBLOCK || head->type == NUMBLOCK) ? "FFFF00" : "FFEFD5"),
        head->type == CONTROLBLOCK ? "diamond" : (head->type == TRUEBLOCK || head->type == FALSEBLOCK) ? "box" :
        ((head->type == IDBLOCK || head->type == NUMBLOCK) ? "plaintext" : head->type == ASSIGNBLOCK ? "larrow" : 
          head->type == LOGOPBLOCK ? "oval" :"circle"));
    int i = 0;
    while (i < head->number_of_children) {
      traverse_ast(head->children[i]);
      fprintf(dot_file, "\t%d -> %d;\n", head->visualization_id, head->children[i++]->visualization_id);
    }
    return;
  }

  void traverse_cfg(Cfg *head, int *visited) {
    if (!head || visited[head->visualization_id])
      return;
    visited[head->visualization_id] = 1;
    char label[1024] = {0}, stmt[1024] = {0};
    sprintf(stmt, "%d", head->program_point);
    sprintf(label, "<V<sub>%s</sub>>", head->program_point == MEET_NODE ? "meet" : (head->program_point ? stmt : "exit"));
    fprintf(dot_file, "\t%d [label=%s,  fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", head->visualization_id, label);
    traverse_cfg(head->out_true, visited);
    traverse_cfg(head->out_false, visited);
    if (head->out_true) {
      stmt[0] = 0;
      fprintf(dot_file, "\t%d -> %d [label=\"%s\"];\n", head->visualization_id, head->out_true->visualization_id,
        expression_to_string(head->statement, stmt));
    }
    if (head->out_false) {
      stmt[0] = 0;
      fprintf(dot_file, "\t%d -> %d [label=\"%s\"];\n", head->visualization_id, head->out_false->visualization_id,
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
    int *visited = (int *) calloc(meet_node_visualization_id, sizeof(int));
    traverse_cfg(cfg, visited);
    free(visited);
  }
  fprintf(dot_file, "\n}");
  fclose(dot_file);
  return;
}