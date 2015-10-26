#include "concrete.h"
#include <stdio.h>

ConcreteStateSet *free_concrete_set(ConcreteStateSet *head) {
  if (!head) return NULL;
  free_concrete_set(head->left);
  free_concrete_set(head->right);
  free(head);
  return NULL;
}

ConcreteStateSet *rotate_right(ConcreteStateSet *y) {
  ConcreteStateSet *x = y->left, *T2 = x->right;
  x->right = y;
  y->left = T2;
  int y_left_height = y->left ? y->left->_height : 0, y_right_height = y->right ? y->right->_height : 0,
      x_left_height = x->left ? x->left->_height : 0, x_right_height = x->right ? x->right->_height : 0;
  y->_height = (y_left_height > y_right_height ? y_left_height : y_right_height) + 1;
  x->_height = (x_left_height > x_right_height ? x_left_height : x_right_height) + 1;
  return x;
}

ConcreteStateSet *rotate_left(ConcreteStateSet *x) {
  ConcreteStateSet *y = x->right, *T2 = y->left;
  y->left = x;
  x->right = T2;
  int y_left_height = y->left ? y->left->_height : 0, y_right_height = y->right ? y->right->_height : 0,
      x_left_height = x->left ? x->left->_height : 0, x_right_height = x->right ? x->right->_height : 0;
  y->_height = (y_left_height > y_right_height ? y_left_height : y_right_height) + 1;
  x->_height = (x_left_height > x_right_height ? x_left_height : x_right_height) + 1;
  return y;
}

ConcreteStateSet *update_concrete_set(ConcreteStateSet *head, int value, double probability, short to_update_existing) {
  ConcreteStateSet *_create_new_node() {
    ConcreteStateSet *node = (ConcreteStateSet *) calloc(1, sizeof(ConcreteStateSet));
    node->state.value = value;
    node->state.probability = probability;
    node->_height = 1;
    return node;
  }

  static short node_found = 0;
  if (!head) return _create_new_node();
  if (head->state.value == value) {
    node_found = 1;
    head->state.probability += to_update_existing ? probability : 0;
    return head;
  }
  else if (head->state.value > value) head->left = update_concrete_set(head->left, value, probability, to_update_existing);
  else head->right = update_concrete_set(head->right, value, probability, to_update_existing);

  int head_left_height = head->left ? head->left->_height : 0,
      head_right_height = head->right ? head->right->_height : 0,
      balance = head ? (head_left_height - head_right_height) : 0;
  head->_height = (head_left_height > head_right_height ? head_left_height : head_right_height) + (node_found ? 0 : 1);
  if (balance > 1 && head->left->state.value > value) return rotate_right(head);
  if (balance < -1 && head->right->state.value < value) return rotate_left(head);
  if (balance > 1 && head->left->state.value < value) {
    head->left = rotate_left(head->left);
    return rotate_right(head);
  }
  if (balance < -1 && head->right->state.value > value) {
    head->right = rotate_right(head->right);
    return rotate_left(head);
  }
  return head;
}





















void avl_to_dot_file(const char *file, ConcreteStateSet *head) {
  FILE *dot_file = fopen(file, "wt");
  void _traverse(ConcreteStateSet *node) {
    if (!node) return;
    fprintf(dot_file, "\t%ld [label=\"%d | %.2lf\", shape=\"record\", fillcolor=\"#FFEFD5\"];\n",
      (unsigned long) node, node->state.value, node->state.probability);
    fprintf(dot_file, "\t%ld -> %ld;\n", (unsigned long) node, node->left ? (unsigned long) node->left : (((unsigned long) node) * 1000));
    fprintf(dot_file, "\t%ld -> %ld;\n", (unsigned long) node, node->right ? (unsigned long) node->right : (((unsigned long) node) * 10000));
    if (!node->left)
      fprintf(dot_file, "\t%ld [label=\"NULL\", shape=\"plaintext\", fillcolor=\"#FFEFD5\"];\n", ((unsigned long) node) * 1000);
    if (!node->right)
      fprintf(dot_file, "\t%ld [label=\"NULL\", shape=\"plaintext\", fillcolor=\"#FFEFD5\"];\n", ((unsigned long) node) * 10000);
    _traverse(node->left);
    _traverse(node->right);
    return;
  }

  fprintf(dot_file, "digraph Concrete_Set {\n\n\trankdir=TB;\n\tnode [style=\"filled\"];\n\n");
  _traverse(head);
  fprintf(dot_file, "\n}");
  fclose(dot_file);
  return;
}











