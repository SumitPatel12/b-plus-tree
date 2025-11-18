#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <variant>

// Forward declarations
template <typename KeyType, std::size_t N> class BTreeInternalNode;
template <typename KeyType, std::size_t N> class BTreeLeafNode;
template <typename KeyData, std::size_t N> class BTree;

// Tagged union for BTree nodes - can be either internal or leaf nodes
template <typename KeyType, std::size_t N>
using BTreeNode = std::variant<BTreeInternalNode<KeyType, N>, BTreeLeafNode<KeyType, N>>;

enum class BTreeNodeType { RootNode, BranchNode, LeafNode };

struct InsertPosition {
  size_t index;
  bool is_duplicate;
};

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

// BTree, can't think of anything other than the root node that would need to be here.
template <typename KeyType, std::size_t N> class BTree {
  BTreeNode<KeyType, N>* root;
};

// Internal node can be root or branch, contains pointers to child nodes.
// Keys are of type KeyType, and a max capacity of N pointers.
template <typename KeyType, std::size_t N> class BTreeInternalNode {
public:
  BTreeInternalNode() : type(BTreeNodeType::BranchNode), numKeys(0) {
    for (std::size_t i = 0; i < N; ++i) {
      children[i] = nullptr;
    }
  }

  BTreeNode<KeyType, N>* children[N];
  KeyType keys[N - 1];
  std::size_t numKeys;
  BTreeNodeType type;

  int insert_key(KeyType key, BTreeNode<KeyType, N>* node);
};

// Leaf node, contains key-pointer pairs pointing to PageData, and pointers to their right sibling.
template <typename KeyType, std::size_t N> class BTreeLeafNode {
public:
  KeyType keys[N - 1];
  PageData* dataPointers[N - 1];
  std::size_t cur_size;
  BTreeLeafNode<KeyType, N>* right_sibling;
  static constexpr BTreeNodeType type = BTreeNodeType::LeafNode;

  BTreeLeafNode() : cur_size(0), right_sibling(nullptr) {
    for (std::size_t i = 0; i < N - 1; ++i) {
      dataPointers[i] = nullptr;
    }
  }

  int insert_key(KeyType key, PageData* page);
};

// I'll have to think more on the return types and the API contract for this one maybe
template <typename KeyType, std::size_t N> int BTreeLeafNode<KeyType, N>::insert_key(KeyType key, PageData* page) {
  // If it's full we need a split so we return -1
  if (cur_size >= (N - 1)) {
    return -1;
  }

  // We don't want duplicates
  const InsertPosition pos = find_index_greater_than_or_equal(keys, cur_size, key);
  // TODO: We need something better maybe?
  if (pos.is_duplicate) {
    return -1;
  }

  // Shift keys and pointers by 1 to make room for the new key
  for (std::size_t i = cur_size; i > pos.index; --i) {
    keys[i] = keys[i - 1];
    dataPointers[i] = dataPointers[i - 1];
  }

  // Now since I've made space for the key, time to insert it.
  keys[pos.index] = key;
  dataPointers[pos.index] = page;
  cur_size++;

  return 0;
}

template <typename DataType>
InsertPosition find_index_greater_than_or_equal(const DataType array[], size_t cur_size, DataType value) {
  const DataType* end = array + cur_size;
  const DataType* pointer = std::lower_bound(array, end, value);

  size_t index = pointer - array; // Works even if pointer == end
  bool is_duplicate = (pointer != end && *pointer == value);

  return {index, is_duplicate};
}
