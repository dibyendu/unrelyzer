#ifndef CONCRETE_H
#define CONCRETE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util.h"

typedef struct ConcreteAllTupleT {
  int symbol_table_index, symbol_table_indices_list_index, current_index, max_values, *values;
} ConcreteAllTuple;

typedef struct ConcreteStateT {
  IntSet *value_set;
  short number_of_values;
  bool is_top_element;
  double probability;
  struct ConcreteStateT **component_states;
} ConcreteState;
 
static bool *empty_state_status, **data_flow_graph;

#endif