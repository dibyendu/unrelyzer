#ifndef CONCRETE_H
#define CONCRETE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util.h"

typedef struct ConcreteStateT {
  IntSet *value_set;
  short number_of_values;
  bool is_top_element;
  double probability;
  struct ConcreteStateT **component_states;
} ConcreteState;
 
static bool *empty_state_status, **data_flow_graph;

#endif