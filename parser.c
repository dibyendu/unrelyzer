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

void store_symbols(const char *token) {
  SYMBOL_TABLE_NODE *tmp = symbol_table;
  while (tmp->next) {
    tmp = tmp->next;
    if (!strcmp(token, tmp->id))
      return;
  }
  tmp->next = (SYMBOL_TABLE_NODE *) calloc(1, sizeof(SYMBOL_TABLE_NODE));
  tmp->next->id = (char *) calloc(strlen(token) + 1, 1);
  strcpy(tmp->next->id, token);
}

/*
 * A tree like structure formed in a bottom up manner
 * where the root node contains the start symbol.
 * All the terminals are stored at the bottom
 * level of the parse tree.
 */
void gen_parse_tree(const char *token, const char *printable, unsigned short is_terminal, unsigned short number_of_children) {
  if (is_terminal) {
    PARSE_TREE_NODE *tmp = (PARSE_TREE_NODE *) calloc(1, sizeof(PARSE_TREE_NODE));
    tmp->token = (char *) calloc(strlen(token) + 1, 1);
    tmp->printable = (char *) calloc(strlen(printable) + 1, 1);
    strcpy(tmp->token, token);
    strcpy(tmp->printable, printable);
    tmp->is_terminal = is_terminal;
    tmp->visualization_id = parse_tree_node_id++;
    parse_stack[parse_stack_top] = tmp;
    parse_stack_top += 1;
    return;
  }
  PARSE_TREE_NODE *tmp = (PARSE_TREE_NODE *) calloc(1, sizeof(PARSE_TREE_NODE));
  tmp->token = (char *) calloc(strlen(token) + 1, 1);
  strcpy(tmp->token, token);
  tmp->number_of_children = number_of_children;
  tmp->children = (PARSE_TREE_NODE **) calloc(number_of_children, sizeof(PARSE_TREE_NODE *));
  tmp->visualization_id = parse_tree_node_id++;
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = parse_stack[--parse_stack_top];
  parse_stack[parse_stack_top++] = tmp;
  return;
}

/*
 * Simple DFS of the parse tree
 */
void pretty_print_parse_tree(PARSE_TREE_NODE *head) {
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

void generate_dot_file(const char *file, const char *graph_name, GRAPH_TYPE type) {
  FILE *dot_file = fopen(file, "wt");

  void traverse_parse_tree(PARSE_TREE_NODE *head) {
    if (head->is_terminal) {
      int i = sizeof(printable_tokens)/sizeof(char *);
      while (i--) {
        if (!strcmp(head->token, printable_tokens[i])) {
          fprintf(dot_file, "\t%hu [label=\"%s | %s\", shape=\"record\", fillcolor=\"#FFEFD5\"];\n",
            head->visualization_id, head->token, head->printable);
          break;
        }
      }
      if (i < 0)
        fprintf(dot_file, "\t%hu [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"box\"];\n", head->visualization_id, head->token);
    }
    else
      fprintf(dot_file, "\t%hu [label=\"%s\", fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", head->visualization_id, head->token);
    int i = 0;
    while (i < head->number_of_children) {
      traverse_parse_tree(head->children[i]);
      fprintf(dot_file, "\t%hu -> %hu;\n", head->visualization_id, head->children[i++]->visualization_id);
    }
    return;
  }

  void traverse_ast(AST_NODE *head) {
    char xlabel[14];
    sprintf(xlabel, "xlabel=\"%hu\"", head->line_number);
    fprintf(dot_file, "\t%hu [label=\"%s\", %s fillcolor=\"#%s\", shape=\"%s\"];\n",
        head->visualization_id, head->token, head->line_number > 0 ? xlabel : "", head->type == TRUEBLOCK ? "00FF00" :
        (head->type == FALSEBLOCK ? "FF0000" : (head->type == IDBLOCK || head->type == NUMBLOCK) ? "FFFF00" : "FFEFD5"),
        head->type == CONTROLBLOCK ? "diamond" : (head->type == TRUEBLOCK || head->type == FALSEBLOCK) ? "box" :
        ((head->type == IDBLOCK || head->type == NUMBLOCK) ? "plaintext" : head->type == ASSIGNBLOCK ? "larrow" : 
          head->type == LOGOPBLOCK ? "oval" :"circle"));
    int i = 0;
    while (i < head->number_of_children) {
      traverse_ast(head->children[i]);
      fprintf(dot_file, "\t%hu -> %hu;\n", head->visualization_id, head->children[i++]->visualization_id);
    }
    return;
  }

  AST_NODE *make_false(AST_NODE *node) {
    if (node->type = LOGOPBLOCK) {
      if (node->number_of_children == 1)
        node = node->children[0];
      else {
        if (!strcmp(node->token, "!="))
          strcpy(node->token, "==");
        else if (!strcmp(node->token, "=="))
          strcpy(node->token, "!=");
        else if (!strcmp(node->token, "<="))
          strcpy(node->token, ">");
        else if (!strcmp(node->token, ">="))
          strcpy(node->token, "<");
        else if (!strcmp(node->token, ">"))
          strcpy(node->token, "<=");
        else if (!strcmp(node->token, "<"))
          strcpy(node->token, ">=");
      }
    }
    return node;
  }

  void printable_expression(AST_NODE *node, char *stmt) {
    if (!node)
      return;
    if (node->number_of_children == 2) {
      strcat(stmt, node->type == ARITHOPBLOCK ? "(" : "");
      printable_expression(node->children[0], stmt);
      strcat(stmt, node->token);
      printable_expression(node->children[1], stmt);
      strcat(stmt, node->type == ARITHOPBLOCK ? ")" : "");
    }
    else if (node->number_of_children == 1) {
      strcat(stmt, "(");
      strcat(stmt, node->token);
      printable_expression(node->children[0], stmt);
      strcat(stmt, ")");
    }
    else {
      strcat(stmt, node->token);
    }
    return;
  }

  void traverse_cfg(CFG_NODE *head, unsigned short *visited) {
    if (!head || visited[head->visualization_id])
      return;
    visited[head->visualization_id] = 1;
    char label[200] = {0}, stmt[200] = {0};
    sprintf(stmt, "%hu", head->program_point);
    sprintf(label, "<V<sub>%s</sub>>", head->program_point == MEET_NODE ? "meet" : (head->program_point ? stmt : "exit"));
    fprintf(dot_file, "\t%hu [label=%s,  fillcolor=\"#FFEFD5\", shape=\"circle\"];\n", head->visualization_id, label);
    traverse_cfg(head->out_true, visited);
    traverse_cfg(head->out_false, visited);
    if (head->out_true) {
      stmt[0] = 0;
      printable_expression(head->statement, stmt);
      fprintf(dot_file, "\t%hu -> %hu [label=\"%s\"];\n", head->visualization_id, head->out_true->visualization_id, stmt);
    }
    if (head->out_false) {
      stmt[0] = 0;
      printable_expression(make_false(head->statement), stmt);
      fprintf(dot_file, "\t%hu -> %hu [label=\"%s\"];\n", head->visualization_id, head->out_false->visualization_id, stmt);
      make_false(head->statement);
    }
    return;
  }

  fprintf(dot_file, "digraph %s {\n\n\trankdir=TB;\n\tnode [style=\"filled\"];\n\n", graph_name);
  if (type == PARSE_TREE)
    traverse_parse_tree(parse_tree);
  else if (type == AST)
    traverse_ast(ast);
  else {
    unsigned short *visited = (unsigned short *) calloc(meet_node_visualization_id, sizeof(unsigned short));
    traverse_cfg(cfg, visited);
    free(visited);
  }
  fprintf(dot_file, "\n}");
  fclose(dot_file);
  return;
}

void gen_ast(const char *token, AST_NODE_TYPE type, unsigned short number_of_children, unsigned short line) {
  AST_NODE *tmp = (AST_NODE *) calloc(1, sizeof(AST_NODE));
  tmp->token = (char *) calloc(strlen(token) + 1, 1);
  strcpy(tmp->token, token);
  tmp->number_of_children = number_of_children;
  tmp->type = type;
  tmp->children = (AST_NODE **) calloc(number_of_children, sizeof(AST_NODE *));
  tmp->visualization_id = ast_node_id++;
  tmp->line_number = line;
  if (line > max_line_number)
    max_line_number = line;
  int i = number_of_children - 1;
  while (number_of_children--)
    tmp->children[i--] = ast_stack[--ast_stack_top];
  ast_stack[ast_stack_top++] = tmp;
  return;
}

void gen_cfg(CFG_NODE *cfg, AST_NODE *ast) {
  static CFG_NODE *last_node = NULL;

  void set_feedback_path(CFG_NODE *cfg, AST_NODE *ast) {
    CFG_NODE *feedback_node;
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

  void handle_control_node(AST_NODE *node) {
    if (!strcmp(node->token, "while")) {
      cfg->statement = node->children[0];
      cfg->program_point = node->line_number;
      cfg_node_bucket[cfg->program_point] = cfg;
      cfg->visualization_id = node->visualization_id;
      last_node = cfg->out_true = (CFG_NODE *) calloc(1, sizeof(CFG_NODE));
      cfg->out_true->in1 = cfg;
      gen_cfg(cfg->out_true, node->children[1]);
      set_feedback_path(cfg, node->children[1]);
      last_node->in1 = cfg;
      cfg = cfg->out_false = last_node;
    }
    else {
      cfg->statement = node->children[0];
      cfg->program_point = node->line_number;
      cfg_node_bucket[cfg->program_point] = cfg;
      cfg->visualization_id = node->visualization_id;
      last_node = cfg->out_true = (CFG_NODE *) calloc(1, sizeof(CFG_NODE));
      cfg->out_true->in1 = cfg;
      gen_cfg(cfg->out_true, node->children[1]);
      CFG_NODE *if_true_last_node = last_node;
      if (node->number_of_children > 2) {
        last_node = cfg->out_false = (CFG_NODE *) calloc(1, sizeof(CFG_NODE));
        cfg->out_false->in1 = cfg;
        gen_cfg(cfg->out_false, node->children[2]);
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
      last_node->out_true = (CFG_NODE *) calloc(1, sizeof(CFG_NODE));
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
    last_node = cfg->out_true = (CFG_NODE *) calloc(1, sizeof(CFG_NODE));
    cfg->out_true->in1 = cfg;
    cfg = cfg->out_true;
    i += 1;
  }
  return;
}