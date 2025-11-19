#include <iostream>
#include <cassert>
#include "btree.h"

// Mock PageData for testing
// We don't need actual data for these tests, just the pointer type
// But since PageData is defined in btree_fwd.h and used, we can just use nullptr or create dummy objects.

void test_insert_empty_tree() {
    std::cout << "Testing insert into empty tree..." << std::endl;
    BTree<int, 3> tree; // Small N for easy testing
    int result = tree.insert(10, nullptr);
    assert(result == 0);
    
    auto* leaf = tree.find(10);
    assert(leaf != nullptr);
    assert(leaf->keys[0] == 10);
    std::cout << "Passed!" << std::endl;
}

void test_insert_multiple() {
    std::cout << "Testing insert multiple keys..." << std::endl;
    BTree<int, 4> tree;
    tree.insert(10, nullptr);
    tree.insert(20, nullptr);
    tree.insert(5, nullptr); // Should be sorted
    
    auto* leaf = tree.find(10);
    assert(leaf != nullptr);
    // Check order in leaf
    assert(leaf->keys[0] == 5);
    assert(leaf->keys[1] == 10);
    assert(leaf->keys[2] == 20);
    std::cout << "Passed!" << std::endl;
}

void test_node_full() {
    std::cout << "Testing node full..." << std::endl;
    BTree<int, 3> tree; // Capacity 3, so max keys = 2
    tree.insert(10, nullptr);
    tree.insert(20, nullptr);
    
    // Third insert should fail (return -1) because split is not implemented
    int result = tree.insert(30, nullptr);
    assert(result == -1);
    std::cout << "Passed!" << std::endl;
}

int main() {
    test_insert_empty_tree();
    test_insert_multiple();
    test_node_full();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
