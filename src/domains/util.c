#include "util.h"

var_t limit(var_t n) {
  return n > MAXINT ? MAXINT : n < MININT ? MININT : n;
}

size_t get_number_of_unique_variables(Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  size_t _get_number_of_unique_variables(Ast *node) {
    if (!node->number_of_children) {
      if (node->type & IDBLOCK) {
        size_t index = symbol_table_entry(node->token);
        if (visited[index]) return 0;
        visited[index] = true;
        return 1;
      }
      return 0;
    }
    size_t i = node->number_of_children, count = 0;
    while (i) count += _get_number_of_unique_variables(node->children[--i]);
    return count;
  }
  
  return _get_number_of_unique_variables(node);
}