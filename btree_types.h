#ifndef BTREE_FWD_H
#define BTREE_FWD_H

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <span>

// Forward declarations
template <typename KeyType, std::size_t N> class BTreeNode;
template <typename KeyType, std::size_t N> class BTreeInternalNode;
template <typename KeyType, std::size_t N> class BTreeLeafNode;
template <typename KeyData, std::size_t N> class BTree;

enum class BTreeNodeType { RootNode, BranchNode, LeafNode };

enum class InsertResult { Success, Duplicate, Full };

struct InsertPosition {
  size_t index;
  bool is_duplicate;
};

// 4KB page data block, that the leaf_nodes points to.
class PageData {
public:
  static constexpr std::size_t PAGE_SIZE = 4096;
  PageData() : data{} {}

  // Rust taught me this :shrug:, mutable and immutable references.
  uint8_t* getData() { return data; }
  const uint8_t* getData() const { return data; }
  uint8_t& operator[](std::size_t index) { return data[index]; }
  const uint8_t& operator[](std::size_t index) const { return data[index]; }
  std::size_t size() const { return PAGE_SIZE; }

private:
  uint8_t data[PAGE_SIZE];
};

// Abstract base class for BTree nodes
template <typename KeyType, std::size_t N> class BTreeNode {
public:
  BTreeNodeType type;
  std::size_t numKeys;

  BTreeNode(BTreeNodeType nodeType) : type(nodeType), numKeys(0) {}
  virtual ~BTreeNode() = default;

  virtual bool isFull() const = 0;
  virtual bool isLeaf() const = 0;
};

template <typename DataType>
InsertPosition find_index_greater_than_or_equal(std::span<const DataType> array, DataType value) {
  auto it = std::ranges::lower_bound(array, value);
  
  size_t index = std::distance(array.begin(), it);
  bool is_duplicate = (it != array.end() && *it == value);

  return {index, is_duplicate};
}

#endif
