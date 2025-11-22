#include "btree.h"
#include <cassert>
#include <iostream>

// Mock PageData for testing
// We don't need actual data for these tests, just the pointer type
// But since PageData is defined in btree_fwd.h and used, we can just use nullptr or create dummy objects.

void test_insert_empty_tree() {
  std::cout << "Testing insert into empty tree..." << std::endl;
  BTree<int, 3> tree; // Small N for easy testing
  InsertResult result = tree.insert(10, nullptr);
  assert(result == InsertResult::Success);

  auto find_res = tree.find(10);
  assert(find_res.leaf_node != nullptr);
  assert(find_res.leaf_node->keys[find_res.idx] == 10);
  std::cout << "Passed!" << std::endl;
}

void test_insert_multiple() {
  std::cout << "Testing insert multiple keys..." << std::endl;
  BTree<int, 4> tree;
  (void)tree.insert(10, nullptr);
  (void)tree.insert(20, nullptr);
  (void)tree.insert(5, nullptr); // Should be sorted

  auto result = tree.find(10);
  assert(result.leaf_node != nullptr);
  auto* leaf = result.leaf_node;
  // Check order in leaf
  assert(leaf->keys[0] == 5);
  assert(leaf->keys[1] == 10);
  assert(leaf->keys[2] == 20);
  std::cout << "Passed!" << std::endl;
}

void test_split_leaf() {
  std::cout << "Testing leaf split..." << std::endl;
  BTree<int, 3> tree; // Capacity 3, so max keys = 2
  (void)tree.insert(10, nullptr);
  (void)tree.insert(20, nullptr);

  // Third insert should cause a split
  InsertResult result = tree.insert(30, nullptr);
  assert(result == InsertResult::Success);

  // Verify split
  // We can't easily access root/structure from outside without friend or public API
  // But we can verify find works for all keys
  assert(tree.find(10).leaf_node != nullptr);
  assert(tree.find(20).leaf_node != nullptr);
  assert(tree.find(30).leaf_node != nullptr);

  // Verify they are in correct leaves (optional, if find works it's good indication)
  auto leaf1 = tree.find(10).leaf_node;
  auto leaf2 = tree.find(30).leaf_node;
  assert(leaf1 != leaf2); // Should be different nodes

  std::cout << "Passed!" << std::endl;
}

void test_split_internal() {
  std::cout << "Testing internal node split (cascading)..." << std::endl;
  BTree<int, 3> tree; // Capacity 3
  // Insert 10, 20, 30 -> Root: [20], L:[10], R:[20, 30]
  (void)tree.insert(10, nullptr);
  (void)tree.insert(20, nullptr);
  (void)tree.insert(30, nullptr);

  // Insert 40 -> R splits. R:[20], R2:[30, 40]. Promoted 30. Root: [20, 30].
  (void)tree.insert(40, nullptr);

  // Insert 50 -> R2 splits. R2:[30], R3:[40, 50]. Promoted 40.
  // Root [20, 30] needs to insert 40. Root full.
  // Root splits. Left:[20]. Right:[40]. Promoted 30.
  // New Root: [30].
  (void)tree.insert(50, nullptr);

  assert(tree.find(10).leaf_node != nullptr);
  assert(tree.find(20).leaf_node != nullptr);
  assert(tree.find(30).leaf_node != nullptr);
  assert(tree.find(40).leaf_node != nullptr);
  assert(tree.find(50).leaf_node != nullptr);

  tree.print();

  std::cout << "Passed!" << std::endl;
}

void test_delete_merge_to_single_leaf() {
  std::cout << "Testing delete causing merge of root's two leaf children..." << std::endl;
  BTree<int, 3> tree; // Capacity 3, max keys = 2, min keys = 1

  // Insert 3 keys to create: Root[20] -> L[10], R[20, 30]
  (void)tree.insert(10, nullptr);
  (void)tree.insert(20, nullptr);
  (void)tree.insert(30, nullptr);

  std::cout << "Tree after insertions:" << std::endl;
  tree.print();

  // Delete 20
  DeletionResult result = tree.delete_key(20);
  assert(result == DeletionResult::Success);

  std::cout << "Tree after deleting 20:" << std::endl;
  tree.print();

  // Delete 30 - right leaf underflows (0 keys), should merge with left
  // Result: single leaf root with [10]
  result = tree.delete_key(30);
  assert(result == DeletionResult::Success);

  std::cout << "Tree after deleting 30 (should merge to single leaf root):" << std::endl;
  tree.print();

  // Verify remaining key
  assert(tree.find(10).leaf_node != nullptr);
  assert(tree.find(20).leaf_node == nullptr);
  assert(tree.find(30).leaf_node == nullptr);

  std::cout << "Passed!" << std::endl;
}

void test_delete_merge_n4() {
  std::cout << "Testing delete causing merge with N=4..." << std::endl;
  BTree<int, 4> tree; // Capacity 4, max keys = 3, min keys = 1

  // Insert 4 keys to cause split: Root[30] -> L[10, 20], R[30]
  (void)tree.insert(10, nullptr);
  (void)tree.insert(20, nullptr);
  (void)tree.insert(30, nullptr);
  (void)tree.insert(40, nullptr);

  std::cout << "Tree after insertions:" << std::endl;
  tree.print();

  // Delete from right leaf to cause underflow and merge
  DeletionResult result = tree.delete_key(30);
  assert(result == DeletionResult::Success);

  std::cout << "Tree after deleting 30:" << std::endl;
  tree.print();

  result = tree.delete_key(40);
  assert(result == DeletionResult::Success);

  std::cout << "Tree after deleting 40 (should merge to single leaf root):" << std::endl;
  tree.print();

  // Verify remaining keys
  assert(tree.find(10).leaf_node != nullptr);
  assert(tree.find(20).leaf_node != nullptr);
  assert(tree.find(30).leaf_node == nullptr);
  assert(tree.find(40).leaf_node == nullptr);

  std::cout << "Passed!" << std::endl;
}

void test_delete_redistribute() {
  std::cout << "Testing delete causing redistribution..." << std::endl;
  BTree<int, 4> tree; // Order 4

  // Insert pattern from b-teee-redistribution.txt
  (void)tree.insert(1, nullptr);
  (void)tree.insert(2, nullptr);
  (void)tree.insert(3, nullptr);
  (void)tree.insert(4, nullptr);
  (void)tree.insert(5, nullptr);
  (void)tree.insert(6, nullptr);
  (void)tree.insert(7, nullptr);
  (void)tree.insert(19, nullptr);
  (void)tree.insert(11, nullptr);
  (void)tree.insert(50, nullptr);
  (void)tree.insert(15, nullptr);
  (void)tree.insert(22, nullptr);

  std::cout << "Tree after insertions:" << std::endl;
  tree.print();

  // Delete 50
  DeletionResult result = tree.delete_key(50);
  assert(result == DeletionResult::Success);

  std::cout << "Tree after deleting 50:" << std::endl;
  tree.print();

  // Delete 19 - should cause redistribution
  result = tree.delete_key(19);
  assert(result == DeletionResult::Success);

  std::cout << "Tree after deleting 19 (should redistribute):" << std::endl;
  tree.print();

  // Verify remaining keys
  assert(tree.find(19).leaf_node == nullptr);
  assert(tree.find(50).leaf_node == nullptr);
  assert(tree.find(15).leaf_node != nullptr);
  assert(tree.find(22).leaf_node != nullptr);
  assert(tree.find(7).leaf_node != nullptr);
  assert(tree.find(11).leaf_node != nullptr);

  std::cout << "Passed!" << std::endl;
}

int main() {
  test_insert_empty_tree();
  test_insert_multiple();
  test_split_leaf();
  test_split_internal();
  test_delete_merge_to_single_leaf();
  test_delete_merge_n4();
  test_delete_redistribute();
  std::cout << "All tests passed!" << std::endl;
  return 0;
}
