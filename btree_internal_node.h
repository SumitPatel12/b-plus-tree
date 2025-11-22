#ifndef BTREE_INTERNAL_NODE_H
#define BTREE_INTERNAL_NODE_H

#include "btree_types.h"
#include <algorithm>
#include <ranges>

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
  
  // Check if node has fewer than minimum required pointers (underflow condition)
  bool isUnderflow() const { return (this->numKeys + 1) < (N + 1) / 2; }  // ceil(N/2) pointers

  InsertResult insert_key(const KeyType& key, BTreeNode<KeyType, N>* node);
  
  // Delete a key and its associated child pointer from the internal node
  void delete_entry(const KeyType& key, BTreeNode<KeyType, N>* child);
};

// Key is inserted with a pointer, we go with index i for key, and i + 1 for the pointer. Likely will populate the first
// pointer when creating the node or something. Not sure right now.
template <typename KeyType, std::size_t N>
InsertResult BTreeInternalNode<KeyType, N>::insert_key(const KeyType& key, BTreeNode<KeyType, N>* node) {
  const InsertPosition pos = find_index_greater_than_or_equal(std::span<const KeyType>(keys, this->numKeys), key);
  if (pos.is_duplicate) {
    return InsertResult::Duplicate;
  }

  if (this->numKeys >= (N - 1)) {
    return InsertResult::Full;
  }

  std::ranges::move_backward(keys + pos.index, keys + this->numKeys, keys + this->numKeys + 1);
  std::ranges::move_backward(children + pos.index + 1, children + this->numKeys + 1, children + this->numKeys + 2);

  keys[pos.index] = key;
  children[pos.index + 1] = node;
  this->numKeys++;

  return InsertResult::Success;
}

// Delete a key and its associated child pointer from internal node
template <typename KeyType, std::size_t N>
void BTreeInternalNode<KeyType, N>::delete_entry(const KeyType& key, BTreeNode<KeyType, N>* /* child */) {
  // Find the key to delete
  auto it = std::ranges::lower_bound(keys, keys + this->numKeys, key);
  
  if (it != keys + this->numKeys && *it == key) {
    std::size_t key_pos = std::distance(keys, it);
    
    // Find the child pointer position (it's at key_pos + 1)
    std::size_t child_pos = key_pos + 1;
    
    // Shift keys left to remove the key
    std::ranges::copy(keys + key_pos + 1, keys + this->numKeys, keys + key_pos);
    
    // Shift children pointers left to remove the child pointer
    std::ranges::copy(children + child_pos + 1, children + this->numKeys + 1, children + child_pos);
    
    this->numKeys--;
  }
}

#endif
