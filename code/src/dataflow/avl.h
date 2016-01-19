#ifndef AVL_H
#define AVL_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct IntSetT {
  int value;
  struct IntSetT *left, *right; 
  int _height;
} IntSet;

#endif