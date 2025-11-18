#ifndef BTREE_INTERNAL_NODE_H
#define BTREE_INTERNAL_NODE_H

#include "btree_fwd.h"

// Internal node can be root or branch, contains pointers to child nodes.
// Keys are of type KeyType, and a max capacity of N pointers.
template <typename KeyType, std::size_t N> class BTreeInternalNode : public BTreeNode<KeyType, N> {
public:
  BTreeInternalNode() : BTreeNode<KeyType, N>(BTreeNodeType::BranchNode) {
    for (std::size_t i = 0; i < N; ++i) {
      children[i] = nullptr;
    }
  }

  BTreeNode<KeyType, N>* children[N];
  KeyType keys[N - 1];

  bool isFull() const override { return this->numKeys >= (N - 1); }
  bool isLeaf() const override { return false; }

  int insert_key(KeyType key, BTreeNode<KeyType, N>* node);
};

// Key is inserted with a pointer, we go with index i for key, and i + 1 for the pointer. Likely will populate the first
// pointer when creating the node or something. Not sure right now.
template <typename KeyType, std::size_t N>
int BTreeInternalNode<KeyType, N>::insert_key(KeyType key, BTreeNode<KeyType, N>* node) {
  if (this->numKeys >= (N - 1)) {
    return -1;
  }

  const InsertPosition pos = find_index_greater_than_or_equal(keys, this->numKeys, key);
  if (pos.is_duplicate) {
    return -1;
  }

  for (std::size_t i = this->numKeys; i > pos.index; --i) {
    keys[i] = keys[i - 1];
    children[i + 1] = children[i];
  }

  keys[pos.index] = key;
  children[pos.index + 1] = node;
  this->numKeys++;

  return 0;
}

#endif
