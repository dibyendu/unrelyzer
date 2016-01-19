#include "util.h"

int limit(int n) {
  return n > MAXINT ? MAXINT : (n < MININT ? MININT : n);
}

unsigned int get_number_of_unique_variables(Ast *node) {
  bool visited[SYMBOL_TABLE_SIZE] = {false};
  int _get_number_of_unique_variables(Ast *node) {
    if (!node->number_of_children) {
      if (is_id_block(node)) {
        int index = get_symbol_table_index(node->token);
        if (visited[index]) return 0;
        visited[index] = true;
        return 1;
      }
      return 0;
    }
    int i = node->number_of_children, count = 0;
    while (i) count += _get_number_of_unique_variables(node->children[--i]);
    return count;
  }
  
  return _get_number_of_unique_variables(node);
}