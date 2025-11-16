#include <cstddef>
#include <cstdint>
#include <variant>

template <typename KeyType, std::size_t N> class BTreeInternalNode;
template <typename KeyType, std::size_t N> class BTreeLeafNode;
template <typename KeyData, std::size_t N> class BTree;

enum class BTreeNodeType { RootNode, BranchNode, LeafNode };

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
template <typename KeyData, std::size_t N> class BTree {
  BTreeInternalNode<KeyData, N>* root;
};

// Internal node can be root or branch, contains pointers to child nodes.
// Keys are of type KeyType, and a max capacity of N pointers.
template <typename KeyType, std::size_t N> class BTreeInternalNode {
public:
  // Tagged unions interesting stuff. Required cause we don't know if the pointer points to a leaf or internal node.
  using ChildPtr = std::variant<BTreeInternalNode<KeyType, N>*, BTreeLeafNode<KeyType, N>*>;

  BTreeInternalNode() : type(BTreeNodeType::BranchNode), numKeys(0) {
    for (std::size_t i = 0; i < N; ++i) {
      children[i] = nullptr;
    }
  }

  ChildPtr children[N];
  KeyType keys[N - 1];
  std::size_t numKeys;
  BTreeNodeType type;
};

// Leaf node, contains key-pointer pairs pointing to PageData, and pointers to their right sibling.
template <typename KeyType, std::size_t N> class BTreeLeafNode {
public:
  BTreeLeafNode() : numKeys(0), right_sibling(nullptr) {
    for (std::size_t i = 0; i < N - 1; ++i) {
      dataPointers[i] = nullptr;
    }
  }

  KeyType keys[N - 1];
  PageData* dataPointers[N - 1];
  std::size_t numKeys;
  BTreeLeafNode<KeyType, N>* right_sibling;
  static constexpr BTreeNodeType type = BTreeNodeType::LeafNode;
};
