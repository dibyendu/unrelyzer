#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "hardware_specification.h"

#define MININT  SHRT_MIN
#define MAXINT  SHRT_MAX
#define MAX_ITERATION 10

#define DEBUG

typedef struct ConcreteValueSetT {
  int value;
  struct ConcreteValueSetT *left, *right; 
  int _height;
} ConcreteValueSet;

typedef struct ConcreteStateT {
  ConcreteValueSet *value_set;
  short number_of_values;
  bool is_top_element;
  double probability;
  struct ConcreteStateT **component_states;
} ConcreteState;
 
bool *empty_state_status, **data_flow_graph;