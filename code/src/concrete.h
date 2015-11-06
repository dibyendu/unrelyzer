#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "hardware_specification.h"

#define MININT -4
#define MAXINT  4

typedef struct ConcreteValueSetT {
  int value;
  struct ConcreteValueSetT *left, *right; 
  int _height;
} ConcreteValueSet;

typedef struct ConcreteStateT {
  ConcreteValueSet *value_set;
  short number_of_values;
  double probability;
  struct ConcreteStateT **component_states;
} ConcreteState;