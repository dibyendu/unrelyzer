#include "cfg.h"

void generate_dataflow_equations() { 
  data_flow_matrix = (Ast ***) calloc((N_stmts + 1), sizeof(Ast **));
  int i;
  for (i = 0; i <= N_stmts; ++i)
    if (cfg_node_bucket[i])
      data_flow_matrix[i] = (Ast **) calloc((N_stmts + 1), sizeof(Ast *));

  void handle_meet_node(Cfg *node, int index) {
    if (node->in1->program_point != MEET_NODE) {
      if (node->in1->out_true && node->in1->out_true == node)
        data_flow_matrix[i][node->in1->program_point] = node->in1->statement;
      else
        data_flow_matrix[i][node->in1->program_point] = invert_expression(node->in1->statement);
    }
    else
      handle_meet_node(node->in1, index);
    if (node->in2->program_point != MEET_NODE) {
      if (node->in2->out_true && node->in2->out_true == node)
        data_flow_matrix[i][node->in2->program_point] = node->in2->statement;
      else
        data_flow_matrix[i][node->in2->program_point] = invert_expression(node->in2->statement);
    }
    else
      handle_meet_node(node->in2, index);
    return;
  }

  for (i = 0; i <= N_stmts; ++i) {
    if (cfg_node_bucket[i]) {
      if (cfg_node_bucket[i]->in1) {
        if (cfg_node_bucket[i]->in1->program_point == MEET_NODE)
          handle_meet_node(cfg_node_bucket[i]->in1, i);
        else {
          if (cfg_node_bucket[i]->in1->out_true && cfg_node_bucket[i]->in1->out_true == cfg_node_bucket[i])
            data_flow_matrix[i][cfg_node_bucket[i]->in1->program_point] = cfg_node_bucket[i]->in1->statement;
          else
            data_flow_matrix[i][cfg_node_bucket[i]->in1->program_point] = invert_expression(cfg_node_bucket[i]->in1->statement);
        }
      }
      if (cfg_node_bucket[i]->in2) {
        if (cfg_node_bucket[i]->in2->program_point == MEET_NODE)
          handle_meet_node(cfg_node_bucket[i]->in2, i);
        else {
          if (cfg_node_bucket[i]->in2->out_true && cfg_node_bucket[i]->in2->out_true == cfg_node_bucket[i])
            data_flow_matrix[i][cfg_node_bucket[i]->in2->program_point] = cfg_node_bucket[i]->in2->statement;
          else
            data_flow_matrix[i][cfg_node_bucket[i]->in2->program_point] = invert_expression(cfg_node_bucket[i]->in2->statement);
        }
      }
    }
  }
  for (i = 0; i <= N_stmts; ++i)
    if (cfg_node_bucket[i])
      free(cfg_node_bucket[i]);
  free(cfg_node_bucket);
  return;
}

void free_dataflow_equations() { 
  int i, j;
  for (i = 0; i <= N_stmts; ++i)
    if (data_flow_matrix[i]) {
      for (j = 0; j <= N_stmts; ++j)
        if (data_flow_matrix[i][j] && data_flow_matrix[i][j]->_is_dataflow_node) {
          if (data_flow_matrix[i][j]->token[0]) free(data_flow_matrix[i][j]->token);
          if (data_flow_matrix[i][j]->children) free(data_flow_matrix[i][j]->children);
          if (data_flow_matrix[i][j]->value) free(data_flow_matrix[i][j]->value);
          free(data_flow_matrix[i][j]);
        }
      free(data_flow_matrix[i]);
    }
  free(data_flow_matrix);
  free(stmt_line_map);
  free_ast(ast);
  free_symbol_table();
  free_function_table();
  free_constant_set(constant_set);
}

Ast *invert_expression(Ast *node) {

  void copy_children(Ast *new_node, Ast *old_node) {
    int i = 0;
    while (i < old_node->number_of_children) {
      new_node->children[i] = old_node->children[i];
      i += 1;
    }
    return;
  }

  void create_false_node(Ast *new_node, Ast *old_node) {
    strcpy(new_node->token, "!");
    new_node->operator_type = NOT;
    new_node->number_of_children = 1;
    new_node->children = (Ast **) calloc(1, sizeof(Ast *));
    new_node->children[0] = old_node;
    return; 
  }

  Ast *new_node = (Ast *) calloc(1, sizeof(Ast));
  new_node->type = LOGOPBLOCK;
  new_node->_is_dataflow_node = true;
  if (node->number_of_children == 1) {
    if (node->token[0] == '!') {
      new_node->token = (char *) calloc(strlen(node->children[0]->token) + 1, sizeof(char));
      strcpy(new_node->token, node->children[0]->token);
      new_node->operator_type = node->children[0]->operator_type;
      new_node->number_of_children = node->children[0]->number_of_children;
      new_node->children = (Ast **) calloc(new_node->number_of_children, sizeof(Ast *));
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
    if (!strcmp(node->token, "!=")) {
      strcpy(new_node->token, "==");
      new_node->operator_type = EQ;
    }
    else if (!strcmp(node->token, "==")) {
      strcpy(new_node->token, "!=");
      new_node->operator_type = NEQ;
    }
    else if (!strcmp(node->token, "<=")) {
      strcpy(new_node->token, ">");
      new_node->operator_type = GT;
    }
    else if (!strcmp(node->token, ">=")) {
      strcpy(new_node->token, "<");
      new_node->operator_type = LT;
    }
    else if (!strcmp(node->token, ">")) {
      strcpy(new_node->token, "<=");
      new_node->operator_type = LEQ;
    }
    else if (!strcmp(node->token, "<")) {
      strcpy(new_node->token, ">=");
      new_node->operator_type = GEQ;
    }
    else {
      create_false_node(new_node, node);
      return new_node;
    }
    new_node->children = (Ast **) calloc(new_node->number_of_children, sizeof(Ast *));
    copy_children(new_node, node);
  }
  else {
    new_node->token = (char *) calloc(2, sizeof(char));
    create_false_node(new_node, node);
  }
  return new_node;
}

char *expression_to_string(Ast *node, char *stmt) {
  if (!node)
    return stmt;
  if (node->number_of_children == 2) {
    strcat(stmt, node->type & ARITHOPBLOCK ? "(" : "");
    expression_to_string(node->children[0], stmt);
    strcat(stmt, node->token);
    expression_to_string(node->children[1], stmt);
    strcat(stmt, node->type & ARITHOPBLOCK ? ")" : "");
  }
  else if (node->number_of_children == 1) {
    strcat(stmt, node->token);
    strcat(stmt, "(");
    expression_to_string(node->children[0], stmt);
    strcat(stmt, ")");
  }
  else
    strcat(stmt, node->token);
  return stmt;
}

void print_dataflow_equations(FILE *stream) {
  int i, j;
  bool first, first_equn = true;
  char stmt[1000] = {0}, subscript[200];
  for (i = 1; i <= N_stmts; i++) {
    if (data_flow_matrix[i]) {
      fprintf(stream, "G%s = %s", print_subsscript(i, subscript), first_equn ? " ⊥" : "");
      first_equn = false;
      first = true;
      for (j = 1; j <= N_stmts; j++) {
        if (data_flow_matrix[i][j]) {
          stmt[0] = 0;
          fprintf(stream, "%ssp(G%s, %s)", first ? " " : " ⊔  ", print_subsscript(j, subscript), expression_to_string(data_flow_matrix[i][j], stmt));
          first = false;
        }
      }
      fprintf(stream, "\n");
    }
  }
  first = true;
  fprintf(stream, "G%s = ", print_subsscript(N_stmts + 1, subscript));
  for (j = 1; j <= N_stmts; j++) {
    if (data_flow_matrix[0][j]) {
      stmt[0] = 0;
      fprintf(stream, "%ssp(G%s, %s)", first ? " " : " ⊔  ", print_subsscript(j, subscript), expression_to_string(data_flow_matrix[0][j], stmt));
      first = false;
    }
  }
  fprintf(stream, "\n");
  return;
}