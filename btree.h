#ifndef BTREE_H
#define BTREE_H

#include "btree_fwd.h"
#include "btree_internal_node.h"
#include "btree_leaf_node.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <ranges>
#include <vector>

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
  InsertResult insert(KeyType key, PageData* data);
  void print() const;

private:
  BTreeLeafNode<KeyType, N>* find_leaf_for_key(KeyType key, std::vector<BTreeNode<KeyType, N>*>& path) const;
  void insert_key_in_parent(BTreeNode<KeyType, N>* node, KeyType key, BTreeNode<KeyType, N>* new_node, std::vector<BTreeNode<KeyType, N>*>& path);
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
    auto it = std::ranges::upper_bound(internalNode->keys, internalNode->keys + internalNode->numKeys, key);
    std::size_t next_pointer_idx = std::distance(internalNode->keys, it);
    cur = internalNode->children[next_pointer_idx];
  }

  auto* leafNode = static_cast<BTreeLeafNode<KeyType, N>*>(cur);
  std::size_t key_idx = std::find(leafNode->keys, leafNode->keys + cur->numKeys, key) - leafNode->keys;
  // If the index was equal to the current size then we didn't find the key, so we return nullptr.
  return key_idx == cur->numKeys ? nullptr : leafNode;
}

template <typename KeyType, std::size_t N>
BTreeLeafNode<KeyType, N>* BTree<KeyType, N>::find_leaf_for_key(KeyType key, std::vector<BTreeNode<KeyType, N>*>& path) const {
  // We'll we got not tree, so no leaf where we can insert the key.
  if (root == nullptr) {
    return nullptr;
  }

  // Loop over the nodes until you find a leaf node.
  BTreeNode<KeyType, N>* cur = root;
  while (!cur->isLeaf()) {
    path.push_back(cur);
    auto* internalNode = static_cast<BTreeInternalNode<KeyType, N>*>(cur);
    auto it = std::ranges::upper_bound(internalNode->keys, internalNode->keys + internalNode->numKeys, key);
    std::size_t next_pointer_idx = std::distance(internalNode->keys, it);
    cur = internalNode->children[next_pointer_idx];
  }

  // return what we found.
  return static_cast<BTreeLeafNode<KeyType, N>*>(cur);
}

template <typename KeyType, std::size_t N> InsertResult BTree<KeyType, N>::insert(KeyType key, PageData* data) {
  // If we've got no tree, we need to make one.
  if (root == nullptr) {
    root = new BTreeLeafNode<KeyType, N>();
  }

  std::vector<BTreeNode<KeyType, N>*> path;
  BTreeLeafNode<KeyType, N>* leaf = find_leaf_for_key(key, path);
  if (!leaf->isFull()) {
    InsertResult result = leaf->insert_key(key, data);
    // TOOD: Handle failure scenarios.
    return result;
  }

  // Split the node
  // Handle the sibling pointers.
  BTreeLeafNode<KeyType, N>* right_node = new BTreeLeafNode<KeyType, N>();
  right_node->right_sibling = leaf->right_sibling;
  leaf->right_sibling = right_node;

  // Since N is the maximum number of pointers a node can accommodate, and N-1 is the upper limit for keys, and we want
  // to move ceil((n-1)/2) which can be dumbed down to N / 2 for integer division.
  size_t split_idx = N / 2;

  // Move keys from split_idx to N-2 (end of current keys) to right_node.
  // Move keys from split_idx to N-2 (end of current keys) to right_node.
  std::ranges::copy(leaf->keys + split_idx, leaf->keys + leaf->numKeys, right_node->keys);
  std::ranges::copy(leaf->dataPointers + split_idx, leaf->dataPointers + leaf->numKeys, right_node->dataPointers);
  
  int moved_count = leaf->numKeys - split_idx;
  // Populate the correct stats for the split nodes.
  right_node->numKeys = moved_count;
  leaf->numKeys = split_idx;

  // Find which node we'd have to insert the new key into.
  // Since we spit we know there's at least one key in the right_node, but we better be sure.
  if (right_node->numKeys > 0 && key >= right_node->keys[0]) {
    right_node->insert_key(key, data);
  } else {
    leaf->insert_key(key, data);
  }

  // Now we've got to propagate the split to the parent.
  insert_key_in_parent(leaf, right_node->keys[0], right_node, path);

  return InsertResult::Success;
}

template <typename KeyType, std::size_t N>
void BTree<KeyType, N>::insert_key_in_parent(BTreeNode<KeyType, N>* node, KeyType key, BTreeNode<KeyType, N>* new_node, std::vector<BTreeNode<KeyType, N>*>& path) {
  if (path.empty()) {
    BTreeInternalNode<KeyType, N>* new_root = new BTreeInternalNode<KeyType, N>();
    new_root->keys[0] = key;
    new_root->children[0] = node;
    new_root->children[1] = new_node;
    new_root->numKeys = 1;
    root = new_root;
    return;
  }

  BTreeNode<KeyType, N>* parent_node = path.back();
  path.pop_back();
  auto* parent = static_cast<BTreeInternalNode<KeyType, N>*>(parent_node);

  if (!parent->isFull()) {
    parent->insert_key(key, new_node);
    return;
  }

  // Split parent
  // Create temp arrays to hold N keys and N+1 pointers
  // (Parent has N-1 keys, N pointers. We add 1 key, 1 pointer. Total N keys, N+1 pointers)
  KeyType temp_keys[N];
  BTreeNode<KeyType, N>* temp_children[N + 1];

  InsertPosition pos = find_index_greater_than_or_equal(parent->keys, parent->numKeys, key);

  // Copy up to pos
  std::ranges::copy(parent->keys, parent->keys + pos.index, temp_keys);
  std::ranges::copy(parent->children, parent->children + pos.index, temp_children);
  temp_children[pos.index] = parent->children[pos.index];

  // Insert new
  temp_keys[pos.index] = key;
  temp_children[pos.index + 1] = new_node;

  // Copy the rest
  std::ranges::copy(parent->keys + pos.index, parent->keys + parent->numKeys, temp_keys + pos.index + 1);
  std::ranges::copy(parent->children + pos.index + 1, parent->children + parent->numKeys + 1, temp_children + pos.index + 2);

  // Split point: N/2
  size_t split_idx = N / 2;
  KeyType promoted_key = temp_keys[split_idx];

  BTreeInternalNode<KeyType, N>* sibling = new BTreeInternalNode<KeyType, N>();

  // Restore parent (Left node)
  parent->numKeys = split_idx;
  std::ranges::copy(temp_keys, temp_keys + split_idx, parent->keys);
  std::ranges::copy(temp_children, temp_children + split_idx + 1, parent->children);

  // Fill sibling (Right node)
  size_t sibling_key_count = N - (split_idx + 1);
  std::ranges::copy(temp_keys + split_idx + 1, temp_keys + N, sibling->keys);
  std::ranges::copy(temp_children + split_idx + 1, temp_children + N + 1, sibling->children);
  sibling->numKeys = sibling_key_count;

  insert_key_in_parent(parent, promoted_key, sibling, path);
}

template <typename KeyType, std::size_t N>
void BTree<KeyType, N>::print() const {
  if (root == nullptr) {
    std::cout << "Empty Tree" << std::endl;
    return;
  }

  std::queue<BTreeNode<KeyType, N>*> q;
  q.push(root);

  int level = 0;
  while (!q.empty()) {
    int level_size = q.size();
    std::cout << "Level " << level << ": ";

    for (int i = 0; i < level_size; ++i) {
      BTreeNode<KeyType, N>* node = q.front();
      q.pop();

      std::cout << "[";
      if (node->isLeaf()) {
        auto* leaf = static_cast<BTreeLeafNode<KeyType, N>*>(node);
        for (size_t j = 0; j < leaf->numKeys; ++j) {
          std::cout << leaf->keys[j] << (j < leaf->numKeys - 1 ? " " : "");
        }
      } else {
        auto* internal = static_cast<BTreeInternalNode<KeyType, N>*>(node);
        for (size_t j = 0; j < internal->numKeys; ++j) {
          std::cout << internal->keys[j] << (j < internal->numKeys - 1 ? " " : "");
        }
        for (size_t j = 0; j <= internal->numKeys; ++j) {
             if (internal->children[j]) q.push(internal->children[j]);
        }
      }
      std::cout << "] ";
    }
    std::cout << std::endl;
    level++;
  }
}

#endif
