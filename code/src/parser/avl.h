#ifndef AVL_H
#define AVL_H

#include <stdlib.h>
#include <stdbool.h>
#include "../limit.h"

typedef struct IntSetT {
  var_t value;
  struct IntSetT *left, *right; 
  long _height;
} IntegerSet;

#endif