#ifndef BTREE_LEAF_NODE_H
#define BTREE_LEAF_NODE_H

#include "btree_fwd.h"

// Leaf node, contains key-pointer pairs pointing to PageData, and pointers to their right sibling.
template <typename KeyType, std::size_t N> class BTreeLeafNode : public BTreeNode<KeyType, N> {
public:
  KeyType keys[N - 1];
  PageData* dataPointers[N - 1];
  BTreeLeafNode<KeyType, N>* right_sibling;

  BTreeLeafNode() : BTreeNode<KeyType, N>(BTreeNodeType::LeafNode), right_sibling(nullptr) {
    for (std::size_t i = 0; i < N - 1; ++i) {
      dataPointers[i] = nullptr;
    }
  }

  bool isFull() const override { return this->numKeys >= (N - 1); }
  bool isLeaf() const override { return true; }

  InsertResult insert_key(KeyType key, PageData* page);
};

// I'll have to think more on the return types and the API contract for this one maybe
template <typename KeyType, std::size_t N> InsertResult BTreeLeafNode<KeyType, N>::insert_key(KeyType key, PageData* page) {
  // If it's full we need a split so we return -1
  if (this->numKeys >= (N - 1)) {
    return InsertResult::Full;
  }

  // We don't want duplicates
  const InsertPosition pos = find_index_greater_than_or_equal(keys, this->numKeys, key);
  // TODO: We need something better maybe?
  if (pos.is_duplicate) {
    return InsertResult::Duplicate;
  }

  // Shift keys and pointers by 1 to make room for the new key
  for (std::size_t i = this->numKeys; i > pos.index; --i) {
    keys[i] = keys[i - 1];
    dataPointers[i] = dataPointers[i - 1];
  }

  // Now since I've made space for the key, time to insert it.
  keys[pos.index] = key;
  dataPointers[pos.index] = page;
  this->numKeys++;

  return InsertResult::Success;
};

#endif
