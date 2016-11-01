#ifndef CFG_H
#define CFG_H

#include "../parser/parser.h"

#define MAX_CFG_NODES 1000

typedef enum CfgNodeT {
  MEET_NODE = -1,
  EXIT_NODE
} CfgNodeT;

typedef struct VisitedCfgNodeT {
  unsigned long ptr;
  bool is_visited;
} VisitedCfgNode;

typedef struct CfgT {
  size_t program_point;
  Ast *statement;
  struct CfgT *out_true, *out_false, *in1, *in2;
} Cfg;

Cfg **cfg_node_bucket, *cfg;
VisitedCfgNode *cfg_node_hash_table;
Ast ***data_flow_matrix;  // data_flow_matrix[N_stmts+1][N_stmts+1];

Ast *invert_expression(Ast *);
char *expression_to_string(Ast *, char *);
void traverse_cfg(void *, FILE *);
void prune_and_rehash_symbol_table(const char *);
Ast *prune_ast(const char *, char **, long **, long **, size_t, Ast *);
void build_control_flow_graph(Cfg *, Ast *);
void generate_dataflow_equations(void);
void print_dataflow_equations(FILE *);

#endif