#include <iostream>
#include <limits>
#include "btree.h"

template <std::size_t N>
void run_demo() {
    BTree<int, N> tree;
    std::cout << "B-Tree Interactive Demo (Order " << N << ")" << std::endl;
    std::cout << "-----------------------" << std::endl;
    std::cout << "Enter an integer to insert it into the tree." << std::endl;
    std::cout << "Enter any non-integer character to quit." << std::endl;

    int key;
    while (true) {
        std::cout << "\n> ";
        if (std::cin >> key) {
            InsertResult result = tree.insert(key, nullptr);
            if (result == InsertResult::Duplicate) {
                std::cout << "Key " << key << " already exists." << std::endl;
            } else if (result == InsertResult::Full) {
                std::cout << "Tree is full (should not happen with splits)." << std::endl;
            } else {
                std::cout << "Inserted " << key << "." << std::endl;
            }
            std::cout << "Current Tree State:" << std::endl;
            tree.print();
        } else {
            // Input was not an integer
            break;
        }
    }
}

int main() {
    int n;
    std::cout << "Enter B-Tree Capacity (N): ";
    if (!(std::cin >> n)) {
        std::cout << "Invalid input. Exiting." << std::endl;
        return 1;
    }

    run_demo<n>();

    std::cout << "Exiting..." << std::endl;
    return 0;
}
