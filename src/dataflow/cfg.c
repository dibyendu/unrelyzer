#include "cfg.h"

Ast *get_param_node(const char *func, Ast *ast) {
  size_t i;
  for (i = 0; i < ast->number_of_children; i++)
    if (ast->children[i]->type & FUNCBLOCK && !strcmp(ast->children[i]->token, func))
      break;
  return (i == ast->number_of_children) ? NULL : ast->children[i]->children[0];
}

bool has_input_error(char **message, const char *func, const char *file, char **params, size_t N_params) {
  Ast *param_node = get_param_node(func, ast);
  *message = (char *) calloc(200, sizeof(char));

  if (!param_node) {
    sprintf(*message, "no function with name ‘%s’ in file ‘%s’", func, file);
    return true;
  }
  else if (param_node->number_of_children > N_params) {
    sprintf(*message, "function ‘%s’ requires %zu integer parameter%s, but only %zu supplied", func, param_node->number_of_children,
    	param_node->number_of_children > 1 ? "s" : "", N_params);
    return true;
  }
  else {
  	size_t i = 0, j;
  	while (i < param_node->number_of_children) {
  		for (j = 0; j < N_params; ++j)
  			if (!strcmp(params[j], param_node->children[i]->token))
  				break;
  		if (j == N_params) {
  			sprintf(*message, "function ‘%s’ requires parameter ‘%s’, but not supplied", func, param_node->children[i]->token);
  			return true;
  		}
  		i += 1;
  	}
  }
  return false;
}

int get_cfg_node_index(unsigned long value) {
  size_t index = number_hash(value, MAX_CFG_NODES);
  while (cfg_node_hash_table[index].ptr) {
    if (cfg_node_hash_table[index].ptr == value) return index;
    index = (index + 1) % MAX_CFG_NODES;
  }
  cfg_node_hash_table[index].ptr = value;
  return index;
}

void init_cfg_data_structures() {
	cfg_node_bucket = (Cfg **) calloc(N_stmts + 1, sizeof(Cfg *));
	cfg = (Cfg *) calloc(1, sizeof(Cfg));
	cfg_node_hash_table = (VisitedCfgNode *) calloc(MAX_CFG_NODES, sizeof(VisitedCfgNode));
	get_cfg_node_index((unsigned long) cfg);
}

void prune_and_rehash_symbol_table(const char *func) {
  size_t index = function_table_entry(func), i, j = 0, deleted = 0;
  for (i = 0; i < N_variables; ++i) {
    if (symbol_table[symbol_table_indices[i]].init_scope[index] == -1 &&
        symbol_table[symbol_table_indices[i]].use_scope[index] == -1 &&
        symbol_table[symbol_table_indices[i]].init_scope[FUNCTION_TABLE_SIZE] == -1) {
      free(symbol_table[symbol_table_indices[i]].id);
      symbol_table[symbol_table_indices[i]].id = NULL;
      free(symbol_table[symbol_table_indices[i]].decl_scope);
      free(symbol_table[symbol_table_indices[i]].init_scope);
      free(symbol_table[symbol_table_indices[i]].use_scope);
      deleted += 1;
      symbol_table_indices[i] = SYMBOL_TABLE_SIZE;
    }
  }
  if (deleted) {
    bool modified_hash_table[SYMBOL_TABLE_SIZE] = {false};
    size_t *new_symbol_table_indices = (size_t *) calloc(N_variables, sizeof(size_t));
    for (i = 0; i < N_variables; ++i) {
      new_symbol_table_indices[i] = SYMBOL_TABLE_SIZE;
      if (symbol_table_indices[i] != SYMBOL_TABLE_SIZE) {
        size_t new_hash_index = string_hash(symbol_table[symbol_table_indices[i]].id, SYMBOL_TABLE_SIZE);
        while (modified_hash_table[new_hash_index])
          new_hash_index = (new_hash_index + 1) % SYMBOL_TABLE_SIZE;
        new_symbol_table_indices[i] = new_hash_index;
      }
    }
    SymbolTable *old_symbol_table = symbol_table;
    symbol_table = (SymbolTable *) calloc(SYMBOL_TABLE_SIZE, sizeof(SymbolTable));
    for (i = 0; i < N_variables; ++i) {
      if (new_symbol_table_indices[i] != SYMBOL_TABLE_SIZE) {
        symbol_table[new_symbol_table_indices[i]].id = old_symbol_table[symbol_table_indices[i]].id;
        symbol_table[new_symbol_table_indices[i]].decl_scope = old_symbol_table[symbol_table_indices[i]].decl_scope;
        symbol_table[new_symbol_table_indices[i]].init_scope = old_symbol_table[symbol_table_indices[i]].init_scope;
        symbol_table[new_symbol_table_indices[i]].use_scope = old_symbol_table[symbol_table_indices[i]].use_scope;
      }
    }
    for (i = 0; i < N_variables; ++i)
      symbol_table_indices[i] = new_symbol_table_indices[i];
    size_t *old_symbol_table_indices = symbol_table_indices;
    symbol_table_indices = (size_t *) calloc(N_variables - deleted, sizeof(size_t));
    for (i = 0; i < N_variables; ++i)
      if (old_symbol_table_indices[i] != SYMBOL_TABLE_SIZE)
        symbol_table_indices[j++] = old_symbol_table_indices[i];
    N_variables -= deleted;
    free(old_symbol_table);
    free(old_symbol_table_indices);
    free(new_symbol_table_indices);
  }
}

Ast *prune_ast(const char *func, char **params, long **value_params, long **interval_params, size_t N_params, Ast *ast) {
  size_t index = function_table_entry(func), i = 0, j = 0, k, deleted = 0;

  bool has_call_or_return_node(Ast *node) {
  	if (!node) return false;
  	if ((node->type & CALLBLOCK) || (node->type & RETURNBLOCK)) return true;
  	int i = node->number_of_children;
  	bool found = false;
    while (i) found |= has_call_or_return_node(node->children[--i]);
    return found;
  }

  Ast *create_ast_node(const char *token, AstType type, OperatorType op, int children, int line, int stmt) {
  	Ast *node = (Ast *) calloc(1, sizeof(Ast));
  	node->token = (char *) calloc(strlen(token) + 1, sizeof(char));
  	strcpy(node->token, token);
  	node->type = type;
  	node->operator_type = op;
  	node->number_of_children = children;
  	node->line_number = line;
  	node->stmt_number = stmt;
  	node->children = (Ast **) calloc(node->number_of_children, sizeof(Ast *));
  	return node;
  }

  void free_ast_node(Ast *node) {
  	free(node->token);
  	free(node->children);
  	free(node);
  }

  while (strcmp(ast->children[i]->token, func)) {
    if (ast->children[i]->type & FUNCBLOCK) {
    	free_ast(ast->children[i]);
      ast->children[i] = NULL;
      deleted += 1;
    }
    i += 1;
  }  
  Ast *func_node = ast->children[i],
  		*param_node = ast->children[i]->children[0],
      *body_node = ast->children[i]->children[1];
  for (j = 0; j < body_node->number_of_children; j++) {
  	if (has_call_or_return_node(body_node->children[j])) {
  		free_ast(body_node->children[j]);
  		body_node->children[j] = NULL;
  		deleted += 1;
  	}
  }
  for (i = i + 1; i < ast->number_of_children; i++) {
    free_ast(ast->children[i]);
    ast->children[i] = NULL;
    deleted += 1;
  }
  for (i = 0; i < param_node->number_of_children; i++) {
  	for (j = 0; j < N_params; j++) {
  		if (!strcmp(params[j], param_node->children[i]->token)) {
  			Ast *node = create_ast_node("=", ASSIGNBLOCK, 0, 2, param_node->children[i]->line_number, param_node->children[i]->stmt_number);
  			param_node->children[i]->line_number = param_node->children[i]->stmt_number = 0;
  			node->children[0] = param_node->children[i];
        char number[40];
        if (value_params[j]) {
          sprintf(number, "%ld", *value_params[j] < 0 ? - *value_params[j] : *value_params[j]);
          if (*value_params[j] < 0) {
            node->children[1] = create_ast_node("-", ARITHOPBLOCK, SUB, 2, 0, 0);
            node->children[1]->children[0] = create_ast_node("0", NUMBLOCK, 0, 0, 0, 0);
            node->children[1]->children[1] = create_ast_node(number, NUMBLOCK, 0, 0, 0, 0);
          }
          else
            node->children[1] = create_ast_node(number, NUMBLOCK, 0, 0, 0, 0);
        }
        else if (interval_params[j]) {
          sprintf(number, "[%ld,%ld]", interval_params[j][0], interval_params[j][1]);
          node->children[1] = create_ast_node(number, INTERVALBLOCK, 0, 0, 0, 0);
        }
  			param_node->children[i] = node;
  			break;
  		}
  	}
  }
  Ast *node = create_ast_node(ast->token, ast->type, ast->operator_type, ast->number_of_children + param_node->number_of_children +
  	body_node->number_of_children - deleted - 1, ast->line_number, ast->stmt_number);
  for (i = 0, j = 0; i < ast->number_of_children; i++)
    if (ast->children[i])
      node->children[j++] = ast->children[i];
  for (i = 0, j = j - 1; i < param_node->number_of_children; i++)
    node->children[j++] = param_node->children[i];
  for (i = 0; i < body_node->number_of_children; i++)
    if (body_node->children[i])
      node->children[j++] = body_node->children[i];
  free_ast_node(func_node);
  free_ast_node(param_node);
  free_ast_node(body_node);
  free_ast_node(ast);
  return node;
}

void build_control_flow_graph(Cfg *cfg, Ast *ast) {
  static Cfg *last_node = NULL;

  void _set_feedback_path(Cfg *cfg, Ast *ast) {
    Cfg *feedback_node;
    if (!strcmp(ast->children[ast->number_of_children - 1]->token, "while")) {
      feedback_node = cfg_node_bucket[ast->children[ast->number_of_children - 1]->stmt_number];
      feedback_node->out_false = cfg;  
    }
    else if (!strcmp(ast->children[ast->number_of_children - 1]->token, "if")) {
      feedback_node = last_node->in1;
      feedback_node->out_true = cfg;
    }
    else {
      feedback_node = cfg_node_bucket[ast->children[ast->number_of_children - 1]->stmt_number];
      feedback_node->out_true = cfg;
    }
    cfg->in2 = feedback_node;
    return;
  }

  void _handle_control_node(Ast *node) {
    if (!strcmp(node->token, "while")) {
      cfg->statement = node->children[0];
      cfg->program_point = node->stmt_number;
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
      cfg->program_point = node->stmt_number;
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
    if (ast->children[i]->type & CONTROLBLOCK) {
      _handle_control_node(ast->children[i]);
      i += 1;
      continue;
    }
    cfg->program_point = ast->children[i]->stmt_number;
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

void traverse_cfg(void *head, FILE *file) {
	void _traverse_cfg(Cfg *head) {
		if (!head || cfg_node_hash_table[get_cfg_node_index((unsigned long) head)].is_visited) return;
		cfg_node_hash_table[get_cfg_node_index((unsigned long) head)].is_visited = true;
		char label[1024] = {0}, stmt[1024] = {0};
		sprintf(stmt, "%zu", head->program_point ? head->program_point : N_stmts + 1);
		sprintf(label, "<V<sub>%s</sub>>", head->program_point == MEET_NODE ? "meet" : stmt);
		fprintf(file, "\t%ld [label=%s, fillcolor=\"#FFFF00\", shape=\"circle\"];\n", (unsigned long) head, label);
		_traverse_cfg(head->out_true);
		_traverse_cfg(head->out_false);
		if (head->out_true) {
			stmt[0] = 0;
			fprintf(file, "\t%ld -> %ld [label=\"%s\"];\n", (unsigned long) head, (unsigned long) head->out_true,
				expression_to_string(head->statement, stmt));
		}
		if (head->out_false) {
			stmt[0] = 0;
			fprintf(file, "\t%ld -> %ld [label=\"%s\"];\n", (unsigned long) head, (unsigned long) head->out_false,
				expression_to_string(invert_expression(head->statement), stmt));
		}
		return;
	}
	_traverse_cfg((Cfg *) head);
}