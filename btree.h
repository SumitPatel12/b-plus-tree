#ifndef BTREE_H
#define BTREE_H

#include "btree_internal_node.h"
#include "btree_leaf_node.h"
#include "btree_types.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
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

  ~BTree();
  [[nodiscard]] FindResult<KeyType, N> find(const KeyType& key) const;
  [[nodiscard]] InsertResult insert(const KeyType& key, PageData* data);
  [[nodiscard]] DeletionResult delete_key(const KeyType& key);
  std::vector<KeyType> find_keys_in_range(const KeyType& lower_bound, const KeyType& upper_bound) const;
  void print() const;

private:
  BTreeLeafNode<KeyType, N>* find_leaf_for_key(const KeyType& key, std::vector<BTreeNode<KeyType, N>*>& path) const;
  void insert_key_in_parent(BTreeNode<KeyType, N>* node, const KeyType& key, BTreeNode<KeyType, N>* new_node,
                            std::vector<BTreeNode<KeyType, N>*>& path);
  void delete_tree(BTreeNode<KeyType, N>* node);

  // Deletion helper methods
  SiblingInfo<KeyType, N> get_sibling(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* parent) const;
  bool can_merge(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* sibling) const;
  void merge_nodes(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* sibling, const KeyType& separator,
                   bool sibling_is_left, BTreeNode<KeyType, N>* parent, std::vector<BTreeNode<KeyType, N>*>& path);
  void redistribute(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* sibling, const KeyType& separator,
                    std::size_t separator_index, bool sibling_is_left, BTreeNode<KeyType, N>* parent);
  void handle_underflow(BTreeNode<KeyType, N>* node, std::vector<BTreeNode<KeyType, N>*>& path);
};

template <typename KeyType, std::size_t N> BTree<KeyType, N>::~BTree() { delete_tree(root); }

template <typename KeyType, std::size_t N> void BTree<KeyType, N>::delete_tree(BTreeNode<KeyType, N>* node) {
  if (node == nullptr) {
    return;
  }

  if (!node->isLeaf()) {
    auto* internalNode = static_cast<BTreeInternalNode<KeyType, N>*>(node);
    for (std::size_t i = 0; i <= internalNode->numKeys; ++i) {
      delete_tree(internalNode->children[i]);
    }
  }

  delete node;
};

// TODO: To keep track of parents of the nodes I'll likely need to keep the pointers in a stack when finding and return
// them, right?
// Find and returns the pointer to the leaf node containing given key. Returns null pointer if key is not found.
template <typename KeyType, std::size_t N> FindResult<KeyType, N> BTree<KeyType, N>::find(const KeyType& key) const {
  std::vector<BTreeNode<KeyType, N>*> path;
  BTreeLeafNode<KeyType, N>* leaf_node = find_leaf_for_key(key, path);

  if (leaf_node == nullptr) {
    return {nullptr, 0};
  }

  auto it = std::ranges::lower_bound(leaf_node->keys, leaf_node->keys + leaf_node->numKeys, key);

  if (it != leaf_node->keys + leaf_node->numKeys && *it == key) {
    std::size_t idx = std::distance(leaf_node->keys, it);
    return {leaf_node, idx};
  }
  return {nullptr, 0};
}

template <typename KeyType, std::size_t N>
BTreeLeafNode<KeyType, N>* BTree<KeyType, N>::find_leaf_for_key(const KeyType& key,
                                                                std::vector<BTreeNode<KeyType, N>*>& path) const {
  // We'll we got not tree, so no leaf where we can insert the key.
  if (root == nullptr) {
    return nullptr;
  }

  // Loop over the nodes until you find a leaf node.
  BTreeNode<KeyType, N>* cur = root;
  while (!cur->isLeaf()) {
    path.push_back(cur);
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

  // return what we found.
  return static_cast<BTreeLeafNode<KeyType, N>*>(cur);
}

template <typename KeyType, std::size_t N> InsertResult BTree<KeyType, N>::insert(const KeyType& key, PageData* data) {
  // If we've got no tree, we need to make one.
  if (root == nullptr) {
    root = new BTreeLeafNode<KeyType, N>();
  }

  std::vector<BTreeNode<KeyType, N>*> path;
  BTreeLeafNode<KeyType, N>* leaf = find_leaf_for_key(key, path);
  InsertResult result = leaf->insert_key(key, data);
  if (result != InsertResult::Full) {
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
void BTree<KeyType, N>::insert_key_in_parent(BTreeNode<KeyType, N>* node, const KeyType& key,
                                             BTreeNode<KeyType, N>* new_node,
                                             std::vector<BTreeNode<KeyType, N>*>& path) {
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

  InsertPosition pos = find_index_greater_than_or_equal(std::span<const KeyType>(parent->keys, parent->numKeys), key);

  // Copy up to pos
  std::ranges::copy(parent->keys, parent->keys + pos.index, temp_keys);
  std::ranges::copy(parent->children, parent->children + pos.index, temp_children);
  temp_children[pos.index] = parent->children[pos.index];

  // Insert new
  temp_keys[pos.index] = key;
  temp_children[pos.index + 1] = new_node;

  // Copy the rest
  std::ranges::copy(parent->keys + pos.index, parent->keys + parent->numKeys, temp_keys + pos.index + 1);
  std::ranges::copy(parent->children + pos.index + 1, parent->children + parent->numKeys + 1,
                    temp_children + pos.index + 2);

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

// Main deletion method - delete a key from the B+ tree
template <typename KeyType, std::size_t N> DeletionResult BTree<KeyType, N>::delete_key(const KeyType& key) {
  if (root == nullptr) {
    return DeletionResult::KeyNotFound;
  }

  std::vector<BTreeNode<KeyType, N>*> path;
  BTreeLeafNode<KeyType, N>* leaf = find_leaf_for_key(key, path);

  if (leaf == nullptr) {
    return DeletionResult::KeyNotFound;
  }

  // Try to delete the key from the leaf node
  if (!leaf->delete_key(key)) {
    return DeletionResult::KeyNotFound;
  }

  // Special case: root is a leaf and is now empty
  if (root == leaf && leaf->numKeys == 0) {
    delete root;
    root = nullptr;
    return DeletionResult::Success;
  }

  // Check if the leaf node underflowed (only if it's not the root)
  if (root != leaf && leaf->isUnderflow()) {
    handle_underflow(leaf, path);
  }

  return DeletionResult::Success;
}

// Get sibling information for a node
template <typename KeyType, std::size_t N>
SiblingInfo<KeyType, N> BTree<KeyType, N>::get_sibling(BTreeNode<KeyType, N>* node,
                                                       BTreeNode<KeyType, N>* parent) const {
  auto* internal_parent = static_cast<BTreeInternalNode<KeyType, N>*>(parent);

  // Find the index of node in parent's children
  std::size_t node_index = 0;
  for (std::size_t i = 0; i <= internal_parent->numKeys; ++i) {
    if (internal_parent->children[i] == node) {
      node_index = i;
      break;
    }
  }

  // Prefer left sibling for consistency
  if (node_index > 0) {
    return {
        internal_parent->children[node_index - 1], // left sibling
        internal_parent->keys[node_index - 1],     // separator key
        node_index - 1,                            // separator index
        true                                       // is_left_sibling
    };
  }

  // Use right sibling if no left sibling
  if (node_index < internal_parent->numKeys) {
    return {
        internal_parent->children[node_index + 1], // right sibling
        internal_parent->keys[node_index],         // separator key
        node_index,                                // separator index
        false                                      // is_left_sibling
    };
  }

  // Should never reach here
  return {nullptr, KeyType{}, 0, false};
}

// Check if two nodes can be merged
template <typename KeyType, std::size_t N>
bool BTree<KeyType, N>::can_merge(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* sibling) const {
  if (node->isLeaf()) {
    // For leaf nodes, check if combined keys fit
    return node->numKeys + sibling->numKeys <= (N - 1);
  } else {
    // For internal nodes, need space for separator key from parent
    return node->numKeys + sibling->numKeys + 1 <= (N - 1);
  }
}

// Merge two nodes into one
template <typename KeyType, std::size_t N>
void BTree<KeyType, N>::merge_nodes(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* sibling,
                                    const KeyType& separator, bool sibling_is_left, BTreeNode<KeyType, N>* parent,
                                    std::vector<BTreeNode<KeyType, N>*>& path) {
  // Normalize: always merge right node into left node
  BTreeNode<KeyType, N>* left_node = sibling_is_left ? sibling : node;
  BTreeNode<KeyType, N>* right_node = sibling_is_left ? node : sibling;

  if (left_node->isLeaf()) {
    // Merge leaf nodes
    auto* left_leaf = static_cast<BTreeLeafNode<KeyType, N>*>(left_node);
    auto* right_leaf = static_cast<BTreeLeafNode<KeyType, N>*>(right_node);

    // Copy all entries from right to left
    std::ranges::copy(right_leaf->keys, right_leaf->keys + right_leaf->numKeys, left_leaf->keys + left_leaf->numKeys);
    std::ranges::copy(right_leaf->dataPointers, right_leaf->dataPointers + right_leaf->numKeys,
                      left_leaf->dataPointers + left_leaf->numKeys);

    left_leaf->numKeys += right_leaf->numKeys;

    // Update sibling pointer: left now points to right's right sibling
    left_leaf->right_sibling = right_leaf->right_sibling;

    delete right_node;
  } else {
    // Merge internal nodes
    auto* left_internal = static_cast<BTreeInternalNode<KeyType, N>*>(left_node);
    auto* right_internal = static_cast<BTreeInternalNode<KeyType, N>*>(right_node);

    // Pull down separator key from parent
    left_internal->keys[left_internal->numKeys] = separator;
    left_internal->numKeys++;

    // Copy all keys and children from right to left
    std::ranges::copy(right_internal->keys, right_internal->keys + right_internal->numKeys,
                      left_internal->keys + left_internal->numKeys);
    std::ranges::copy(right_internal->children, right_internal->children + right_internal->numKeys + 1,
                      left_internal->children + left_internal->numKeys);

    left_internal->numKeys += right_internal->numKeys;

    delete right_node;
  }

  // Delete the separator and pointer from parent
  auto* internal_parent = static_cast<BTreeInternalNode<KeyType, N>*>(parent);
  internal_parent->delete_entry(separator, right_node);

  // Special case: if parent is root and now empty, make left_node the new root
  if (parent == root && internal_parent->numKeys == 0) {
    root = left_node;
    delete parent;
  } else if (parent != root && internal_parent->isUnderflow()) {
    // Parent might now underflow, handle recursively
    handle_underflow(parent, path);
  }
}

// Redistribute entries between node and sibling
template <typename KeyType, std::size_t N>
void BTree<KeyType, N>::redistribute(BTreeNode<KeyType, N>* node, BTreeNode<KeyType, N>* sibling,
                                     const KeyType& separator, std::size_t separator_index, bool sibling_is_left,
                                     BTreeNode<KeyType, N>* parent) {
  auto* internal_parent = static_cast<BTreeInternalNode<KeyType, N>*>(parent);

  if (sibling_is_left) {
    // Borrow from left sibling
    if (node->isLeaf()) {
      auto* leaf = static_cast<BTreeLeafNode<KeyType, N>*>(node);
      auto* sibling_leaf = static_cast<BTreeLeafNode<KeyType, N>*>(sibling);

      std::size_t borrow_idx = sibling_leaf->numKeys - 1;

      // Shift node's entries right to make room
      std::ranges::move_backward(leaf->keys, leaf->keys + leaf->numKeys, leaf->keys + leaf->numKeys + 1);
      std::ranges::move_backward(leaf->dataPointers, leaf->dataPointers + leaf->numKeys,
                                 leaf->dataPointers + leaf->numKeys + 1);

      // Move last entry from sibling to first position in node
      leaf->keys[0] = sibling_leaf->keys[borrow_idx];
      leaf->dataPointers[0] = sibling_leaf->dataPointers[borrow_idx];
      leaf->numKeys++;
      sibling_leaf->numKeys--;

      // Update separator in parent to node's new first key
      internal_parent->keys[separator_index] = leaf->keys[0];
    } else {
      auto* internal = static_cast<BTreeInternalNode<KeyType, N>*>(node);
      auto* sibling_internal = static_cast<BTreeInternalNode<KeyType, N>*>(sibling);

      std::size_t borrow_key_idx = sibling_internal->numKeys - 1;

      // Shift node's entries right
      std::ranges::move_backward(internal->keys, internal->keys + internal->numKeys,
                                 internal->keys + internal->numKeys + 1);
      std::ranges::move_backward(internal->children, internal->children + internal->numKeys + 1,
                                 internal->children + internal->numKeys + 2);

      // Bring down separator from parent as first key in node
      internal->keys[0] = separator;
      internal->children[0] = sibling_internal->children[sibling_internal->numKeys];
      internal->numKeys++;

      // Move last key from sibling up to parent
      internal_parent->keys[separator_index] = sibling_internal->keys[borrow_key_idx];
      sibling_internal->numKeys--;
    }
  } else {
    // Borrow from right sibling
    if (node->isLeaf()) {
      auto* leaf = static_cast<BTreeLeafNode<KeyType, N>*>(node);
      auto* sibling_leaf = static_cast<BTreeLeafNode<KeyType, N>*>(sibling);

      // Move first entry from sibling to last position in node
      leaf->keys[leaf->numKeys] = sibling_leaf->keys[0];
      leaf->dataPointers[leaf->numKeys] = sibling_leaf->dataPointers[0];
      leaf->numKeys++;

      // Shift sibling's entries left
      std::ranges::copy(sibling_leaf->keys + 1, sibling_leaf->keys + sibling_leaf->numKeys, sibling_leaf->keys);
      std::ranges::copy(sibling_leaf->dataPointers + 1, sibling_leaf->dataPointers + sibling_leaf->numKeys,
                        sibling_leaf->dataPointers);
      sibling_leaf->numKeys--;

      // Update separator in parent to sibling's new first key
      internal_parent->keys[separator_index] = sibling_leaf->keys[0];
    } else {
      auto* internal = static_cast<BTreeInternalNode<KeyType, N>*>(node);
      auto* sibling_internal = static_cast<BTreeInternalNode<KeyType, N>*>(sibling);

      // Bring down separator from parent as last key in node
      internal->keys[internal->numKeys] = separator;
      internal->children[internal->numKeys + 1] = sibling_internal->children[0];
      internal->numKeys++;

      // Move first key from sibling up to parent
      internal_parent->keys[separator_index] = sibling_internal->keys[0];

      // Shift sibling's entries left
      std::ranges::copy(sibling_internal->keys + 1, sibling_internal->keys + sibling_internal->numKeys,
                        sibling_internal->keys);
      std::ranges::copy(sibling_internal->children + 1, sibling_internal->children + sibling_internal->numKeys + 1,
                        sibling_internal->children);
      sibling_internal->numKeys--;
    }
  }
}

// Handle underflow by redistributing or merging
template <typename KeyType, std::size_t N>
void BTree<KeyType, N>::handle_underflow(BTreeNode<KeyType, N>* node, std::vector<BTreeNode<KeyType, N>*>& path) {
  // Special case: node is root
  if (path.empty()) {
    if (!node->isLeaf() && node->numKeys == 0) {
      // Root has only one child, make it the new root
      auto* internal_root = static_cast<BTreeInternalNode<KeyType, N>*>(node);
      root = internal_root->children[0];
      delete node;
    }
    return;
  }

  BTreeNode<KeyType, N>* parent = path.back();
  path.pop_back();

  // Get sibling information
  SiblingInfo<KeyType, N> sib_info = get_sibling(node, parent);

  // Check if we can merge
  if (can_merge(node, sib_info.sibling)) {
    merge_nodes(node, sib_info.sibling, sib_info.separator_key, sib_info.is_left_sibling, parent, path);
  } else {
    // Redistribute entries
    redistribute(node, sib_info.sibling, sib_info.separator_key, sib_info.separator_index, sib_info.is_left_sibling,
                 parent);
  }
}

template <typename KeyType, std::size_t N> void BTree<KeyType, N>::print() const {
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

// Find keys in range: [lower_bound, upper_bound)
template <typename KeyType, std::size_t N>
std::vector<KeyType> BTree<KeyType, N>::find_keys_in_range(const KeyType& lower_bound,
                                                           const KeyType& upper_bound) const {
  std::vector<KeyType> result;
  std::vector<BTreeNode<KeyType, N>*> path;
  // Find the leaf_node that is supposed to house the lower_bound.
  BTreeLeafNode<KeyType, N>* leaf_node = find_leaf_for_key(lower_bound, path);

  if (leaf_node == nullptr) {
    return result;
  }

  auto it = std::ranges::lower_bound(leaf_node->keys, leaf_node->keys + leaf_node->numKeys, lower_bound);
  std::size_t idx = std::distance(leaf_node->keys, it);

  while (leaf_node != nullptr) {
    // If we reached the end of the current node, move to the next sibling
    if (idx >= leaf_node->numKeys) {
      leaf_node = leaf_node->right_sibling;
      idx = 0;
      continue;
    }

    // If the current key is out of range, we are done
    if (leaf_node->keys[idx] >= upper_bound) {
      break;
    }

    result.push_back(leaf_node->keys[idx]);
    idx++;
  }

  return result;
}

#endif
