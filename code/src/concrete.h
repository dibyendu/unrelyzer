#include <stdlib.h>
#include "hardware_specification.h"

#define MININT 0
#define MAXINT 3

typedef struct ConcreteStateT {
  int value;
  double probability;
} ConcreteState;

typedef struct ConcreteStateSetT {
  ConcreteState state;
  struct ConcreteStateSetT *left, *right;
  int _height;
} ConcreteStateSet;

ConcreteStateSet *free_concrete_set(ConcreteStateSet *);
ConcreteStateSet *update_concrete_set(ConcreteStateSet *, int, double, short);

void avl_to_dot_file(const char *, ConcreteStateSet *);