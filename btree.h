#ifndef BTREE_H
#define BTREE_H

#include "btree_fwd.h"
#include "btree_internal_node.h"
#include "btree_leaf_node.h"

/*
 * What do I want as an API in the BTree?
 *  1. Insertion
 *  2. Deletion
 *  3. Search Keys within range
 *  4. Splits
 *  5. Merges
 *
 * What would be the helper methods for the nodes?
 *  1. Create a node, either BTreeInternalNode, or BTreeLeafNode
 *  2. Add a key to the keys array in the node, preserving the key sort order.
 *  3. Add a pointer in the pointers array, once again preserving the order.
 *  4. Get parent of the node? I will likely need this, but how am I to find the parent without much overhead or
 *  traversing the tree? Maybe a stack that keeps track of all the pointers/nodes I've traversed.
 */

// BTree, can't think of anything other than the root node that would need to be here.
template <typename KeyType, std::size_t N> class BTree {
  BTreeNode<KeyType, N>* root;

public:
  BTree() : root(nullptr) {}

  BTreeLeafNode<KeyType, N>* find(KeyType key) const;
  int insert(KeyType key, PageData* data);

private:
  BTreeLeafNode<KeyType, N>* find_leaf_for_key(KeyType key) const;
};

// TODO: To keep track of parents of the nodes I'll likely need to keep the pointers in a stack when finding and return
// them, right?
// Find and returns the pointer to the leaf node containing given key. Returns null pointer if key is not found.
template <typename KeyType, std::size_t N> BTreeLeafNode<KeyType, N>* BTree<KeyType, N>::find(KeyType key) const {
  if (root == nullptr) {
    return nullptr;
  }

  BTreeNode<KeyType, N>* cur = root;
  // I need to iterate over nodes util I reach a leaf node. Right.
  while (!cur->isLeaf()) {
    // Get the first key index that is greater than the key we're looking for, since there are going to be no duplicates
    // we don't need to worry about equals param.
    // The one we find will be the pointer we follow. For instance consider a capacity of 5 pointers, 4 keys. With a
    // node looking like this:
    // . 4 . 18 . 30 . 50 .     (. represents a pointer)
    //
    // We need to find a pointer that would lead us to a leaf containing the value 20. It would fall b/w 18 and 30, idx
    // of 30 is 2, so would be the idx of the desired pointer.
    //
    // Now consider the case where its greater than the  greatest/last key, then we'd get idx = 4 which is once again
    // correct.
    //
    // If the first element itself is greater than the key then we know it should be the first pointer and that's what
    // upper_bound will return so thats once again correct.
    auto* internalNode = static_cast<BTreeInternalNode<KeyType, N>*>(cur);
    std::size_t next_pointer_idx =
        std::upper_bound(internalNode->keys, internalNode->keys + internalNode->numKeys, key) - internalNode->keys;
    cur = internalNode->children[next_pointer_idx];
  }

  auto* leafNode = static_cast<BTreeLeafNode<KeyType, N>*>(cur);
  std::size_t key_idx = std::find(leafNode->keys, leafNode->keys + cur->numKeys, key) - leafNode->keys;
  // If the index was equal to the current size then we didn't find the key, so we return nullptr.
  return key_idx == cur->numKeys ? nullptr : leafNode;
}

template <typename KeyType, std::size_t N>
BTreeLeafNode<KeyType, N>* BTree<KeyType, N>::find_leaf_for_key(KeyType key) const {
  if (root == nullptr) {
    return nullptr;
  }

  BTreeNode<KeyType, N>* cur = root;
  while (!cur->isLeaf()) {
    auto* internalNode = static_cast<BTreeInternalNode<KeyType, N>*>(cur);
    std::size_t next_pointer_idx =
        std::upper_bound(internalNode->keys, internalNode->keys + internalNode->numKeys, key) - internalNode->keys;
    cur = internalNode->children[next_pointer_idx];
  }

  return static_cast<BTreeLeafNode<KeyType, N>*>(cur);
}

template <typename KeyType, std::size_t N> int BTree<KeyType, N>::insert(KeyType key, PageData* data) {
  if (root == nullptr) {
    root = new BTreeLeafNode<KeyType, N>();
  }

  BTreeLeafNode<KeyType, N>* leaf = find_leaf_for_key(key);
  int result = leaf->insert_key(key, data);

  if (result == -1) {
    // TODO: Handle split
  }

  return result;
}

#endif
