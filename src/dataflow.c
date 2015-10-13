#include "parser.h"

void gen_dataflow_eqn() { 
  data_flow_matrix = (AST_NODE ***) calloc((max_line_number + 1), sizeof(AST_NODE **));
  short i;
  for (i = 0; i < max_line_number + 1; ++i)
    if (cfg_node_bucket[i])
      data_flow_matrix[i] = (AST_NODE **) calloc((max_line_number + 1), sizeof(AST_NODE *));

  void handle_meet_node(CFG_NODE *node, short index) {
    if (node->in1->program_point != MEET_NODE) {
      if (node->in1->out_true && node->in1->out_true == node)
        data_flow_matrix[i][node->in1->program_point] = node->in1->statement;
      else
        data_flow_matrix[i][node->in1->program_point] = make_expression_false(node->in1->statement);
    }
    else
      handle_meet_node(node->in1, index);
    if (node->in2->program_point != MEET_NODE) {
      if (node->in2->out_true && node->in2->out_true == node)
        data_flow_matrix[i][node->in2->program_point] = node->in2->statement;
      else
        data_flow_matrix[i][node->in2->program_point] = make_expression_false(node->in2->statement);
    }
    else
      handle_meet_node(node->in2, index);
    return;
  }

  for (i = 0; i < max_line_number + 1; ++i) {
    if (cfg_node_bucket[i]) {
      if (cfg_node_bucket[i]->in1) {
        if (cfg_node_bucket[i]->in1->program_point == MEET_NODE)
          handle_meet_node(cfg_node_bucket[i]->in1, i);
        else {
          if (cfg_node_bucket[i]->in1->out_true && cfg_node_bucket[i]->in1->out_true == cfg_node_bucket[i])
            data_flow_matrix[i][cfg_node_bucket[i]->in1->program_point] = cfg_node_bucket[i]->in1->statement;
          else
            data_flow_matrix[i][cfg_node_bucket[i]->in1->program_point] = make_expression_false(cfg_node_bucket[i]->in1->statement);
        }
      }
      if (cfg_node_bucket[i]->in2) {
        if (cfg_node_bucket[i]->in2->program_point == MEET_NODE)
          handle_meet_node(cfg_node_bucket[i]->in2, i);
        else {
          if (cfg_node_bucket[i]->in2->out_true && cfg_node_bucket[i]->in2->out_true == cfg_node_bucket[i])
            data_flow_matrix[i][cfg_node_bucket[i]->in2->program_point] = cfg_node_bucket[i]->in2->statement;
          else
            data_flow_matrix[i][cfg_node_bucket[i]->in2->program_point] = make_expression_false(cfg_node_bucket[i]->in2->statement);
        }
      }
    }
  }
  return;
}

AST_NODE *make_expression_false(AST_NODE *node) {

  void copy_children(AST_NODE *new_node, AST_NODE *old_node) {
    int i = 0;
    while (i < old_node->number_of_children) {
      new_node->children[i] = old_node->children[i];
      i += 1;
    }
    return;
  }

  void create_false_node(AST_NODE *new, AST_NODE *old) {
    strcpy(new->token, "!");
    new->number_of_children = 1;
    new->children = (AST_NODE **) calloc(1, sizeof(AST_NODE *));
    new->children[0] = old;
    return; 
  }

  AST_NODE *new_node = (AST_NODE *) calloc(1, sizeof(AST_NODE));
  new_node->type = LOGOPBLOCK;
  if (node->number_of_children == 1) {
    if (node->token[0] == '!') {
      new_node->token = (char *) calloc(strlen(node->children[0]->token) + 1, sizeof(char));
      strcpy(new_node->token, node->children[0]->token);
      new_node->number_of_children = node->children[0]->number_of_children;
      new_node->children = (AST_NODE **) calloc(new_node->number_of_children, sizeof(AST_NODE *));
      copy_children(new_node, node->children[0]);
    }
    else {
      new_node->token = (char *) calloc(2, sizeof(char));
      create_false_node(new_node, node);
    }
  }
  else if (node->number_of_children == 2){
    new_node->token = (char *) calloc(3, sizeof(char));
    new_node->number_of_children = 2;
    if (!strcmp(node->token, "!="))
      strcpy(new_node->token, "==");
    else if (!strcmp(node->token, "=="))
      strcpy(new_node->token, "!=");
    else if (!strcmp(node->token, "<="))
      strcpy(new_node->token, ">");
    else if (!strcmp(node->token, ">="))
      strcpy(new_node->token, "<");
    else if (!strcmp(node->token, ">"))
      strcpy(new_node->token, "<=");
    else if (!strcmp(node->token, "<"))
      strcpy(new_node->token, ">=");
    else {
      create_false_node(new_node, node);
      return new_node;
    }
    new_node->children = (AST_NODE **) calloc(new_node->number_of_children, sizeof(AST_NODE *));
    copy_children(new_node, node);
  }
  else {
    new_node->token = (char *) calloc(2, sizeof(char));
    create_false_node(new_node, node);
  }
  return new_node;
}

char *expression_to_string(AST_NODE *node, char *stmt) {
  if (!node)
    return stmt;
  if (node->number_of_children == 2) {
    strcat(stmt, node->type == ARITHOPBLOCK ? "(" : "");
    expression_to_string(node->children[0], stmt);
    strcat(stmt, node->token);
    expression_to_string(node->children[1], stmt);
    strcat(stmt, node->type == ARITHOPBLOCK ? ")" : "");
  }
  else if (node->number_of_children == 1) {
    strcat(stmt, "(");
    strcat(stmt, node->token);
    expression_to_string(node->children[0], stmt);
    strcat(stmt, ")");
  }
  else
    strcat(stmt, node->token);
  return stmt;
}

void print_dataflow_eqn() {
  short i, j, first;
  char stmt[1024] = {0};
  for (i = 1; i < max_line_number + 1; i++) {
    if (data_flow_matrix[i]) {
      printf("g%hd = ", i);
      first = 1;
      for (j = 1; j < max_line_number + 1; j++) {
        if (data_flow_matrix[i][j]) {
          stmt[0] = 0;
          printf("%ssp(g%hd, %s)", first ? " " : " U ", j, expression_to_string(data_flow_matrix[i][j], stmt));
          first = first ? 0 : 0;
        }
      }
      printf("\n");
    }
  }
  first = 1;
  printf("g_exit = ");
  for (j = 1; j < max_line_number + 1; j++) {
    if (data_flow_matrix[0][j]) {
      stmt[0] = 0;
      printf("%ssp(g%hd, %s)", first ? " " : " U ", j, expression_to_string(data_flow_matrix[0][j], stmt));
      first = first ? 0 : 0;
    }
  }
  printf("\n");
  return;
}