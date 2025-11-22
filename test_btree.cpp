#include <iostream>
#include <cassert>
#include "btree.h"

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
    tree.insert(10, nullptr);
    tree.insert(20, nullptr);
    tree.insert(5, nullptr); // Should be sorted
    
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
    tree.insert(10, nullptr);
    tree.insert(20, nullptr);
    
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
    tree.insert(10, nullptr);
    tree.insert(20, nullptr);
    tree.insert(30, nullptr);
    
    // Insert 40 -> R splits. R:[20], R2:[30, 40]. Promoted 30. Root: [20, 30].
    tree.insert(40, nullptr);
    
    // Insert 50 -> R2 splits. R2:[30], R3:[40, 50]. Promoted 40.
    // Root [20, 30] needs to insert 40. Root full.
    // Root splits. Left:[20]. Right:[40]. Promoted 30.
    // New Root: [30].
    tree.insert(50, nullptr);
    
    assert(tree.find(10).leaf_node != nullptr);
    assert(tree.find(20).leaf_node != nullptr);
    assert(tree.find(30).leaf_node != nullptr);
    assert(tree.find(40).leaf_node != nullptr);
    assert(tree.find(50).leaf_node != nullptr);
    
    tree.print();

    std::cout << "Passed!" << std::endl;
}

int main() {
    test_insert_empty_tree();
    test_insert_multiple();
    test_split_leaf();
    test_split_internal();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
