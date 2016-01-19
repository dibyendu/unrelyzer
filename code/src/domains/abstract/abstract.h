#ifndef ABSTRACT_H
#define ABSTRACT_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util.h"

typedef struct AbstractStateT {
  int lower, upper;
  bool is_empty_interval;
  double probability;
  struct AbstractStateT **component_states;
} AbstractState;

static bool *empty_state_status, **data_flow_graph;

#endif
