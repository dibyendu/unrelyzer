#include "avl.h"

IntegerSet *avl_tree_insert(IntegerSet *head, var_t value, bool *is_key_found) {
  int height(IntegerSet *node) {
    return node ? node->_height : -1;
  }
  IntegerSet *_single_rotate_with_left(IntegerSet *x) {
    IntegerSet *y = x->left;
    x->left = y->right;
    y->right = x;
    x->_height = (height(x->left) > height(x->right) ? height(x->left) : height(x->right)) + 1;
    y->_height = (height(y->left) > x->_height ? height(y->left) : x->_height) + 1;
    return y;
  }
  IntegerSet *_single_rotate_with_right(IntegerSet *x) {
    IntegerSet *y = x->right;
    x->right = y->left;
    y->left = x;
    x->_height = (height(x->left) > height(x->right) ? height(x->left) : height(x->right)) + 1;
    y->_height = (height(y->right) > x->_height ? height(y->right) : x->_height) + 1;
    return y;
  }
  IntegerSet *_double_rotate_with_left(IntegerSet *x) {
    x->left = _single_rotate_with_right(x->left);
    return _single_rotate_with_left(x);
  }
  IntegerSet *_double_rotate_with_right(IntegerSet *x) {
    x->right = _single_rotate_with_left(x->right);
    return _single_rotate_with_right(x);
  }

  if (!head) {
    head = (IntegerSet *) calloc(1, sizeof(IntegerSet));
    head->value = value;
    head->_height = 0;
  }
  else if (value < head->value) {
    head->left  = avl_tree_insert(head->left, value, is_key_found);
    if (height(head->left) - height(head->right) == 2 ) {
      if (value < head->left->value) head = _single_rotate_with_left(head);
      else head = _double_rotate_with_left(head);
    }
  }
  else if (value > head->value) {
    head->right  = avl_tree_insert(head->right, value, is_key_found);
    if (height(head->right) - height(head->left) == 2 ) {
      if (value > head->right->value) head = _single_rotate_with_right(head);
      else head = _double_rotate_with_right(head);
    }
  }
  else
    *is_key_found = true;
  head->_height = (height(head->left) > height(head->right) ? height(head->left) : height(head->right)) + 1;
  return head;
}