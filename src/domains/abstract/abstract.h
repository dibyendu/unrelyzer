#ifndef ABSTRACT_H
#define ABSTRACT_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util.h"

typedef struct AbstractAllTupleT {
  size_t symbol_table_index, symbol_table_indices_list_index, current_index;
  unsigned long max_values;
  var_t min_value;
} AbstractAllTuple;

typedef struct AbstractStateT {
  var_t lower, upper;
  bool is_empty_interval;
  long double probability;
  struct AbstractStateT **component_states;
} AbstractState;

static bool *empty_state_status, **data_flow_graph;

#endif
