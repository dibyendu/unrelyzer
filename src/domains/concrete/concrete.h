#ifndef CONCRETE_H
#define CONCRETE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util.h"

typedef struct ConcreteAllTupleT {
  size_t symbol_table_index, symbol_table_indices_list_index, current_index;
  unsigned long max_values;
  var_t *values;
} ConcreteAllTuple;

typedef struct ConcreteStateT {
  IntegerSet *value_set;
  size_t number_of_values;
  bool is_top_element;
  long double probability;
  struct ConcreteStateT **component_states;
} ConcreteState;
 
static bool *empty_state_status, **data_flow_graph;

#endif