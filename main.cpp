#include <iostream>
#include <limits>
#include <string>
#include "btree.h"

template <std::size_t N>
void run_demo() {
    BTree<int, N> tree;
    std::cout << "B+ Tree Interactive Demo (Order " << N << ")" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  i <key> - Insert a key" << std::endl;
    std::cout << "  d <key> - Delete a key" << std::endl;
    std::cout << "  q       - Quit" << std::endl;
    std::cout << "========================================" << std::endl;

    std::string command;
    int key;
    
    while (true) {
        std::cout << "\n> ";
        if (!(std::cin >> command)) {
            break;
        }
        
        if (command == "q" || command == "quit") {
            break;
        } else if (command == "i" || command == "insert") {
            if (!(std::cin >> key)) {
                std::cout << "Invalid key. Please enter an integer." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            
            InsertResult result = tree.insert(key, nullptr);
            if (result == InsertResult::Duplicate) {
                std::cout << "Key " << key << " already exists." << std::endl;
            } else if (result == InsertResult::Full) {
                std::cout << "Tree is full (should not happen with splits)." << std::endl;
            } else {
                std::cout << "Inserted " << key << std::endl;
            }
            
            std::cout << "\nCurrent Tree State:" << std::endl;
            tree.print();
            
        } else if (command == "d" || command == "delete") {
            if (!(std::cin >> key)) {
                std::cout << "Invalid key. Please enter an integer." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            
            DeletionResult result = tree.delete_key(key);
            if (result == DeletionResult::KeyNotFound) {
                std::cout << "Key " << key << " not found." << std::endl;
            } else {
                std::cout << "Deleted " << key << std::endl;
            }
            
            std::cout << "\nCurrent Tree State:" << std::endl;
            tree.print();
            
        } else {
            std::cout << "Unknown command. Use 'i <key>' to insert, 'd <key>' to delete, or 'q' to quit." << std::endl;
        }
    }
}

int main() {
    std::cout << "Enter B+ Tree Order (N): ";
    int n;
    if (!(std::cin >> n) || n < 3) {
        std::cout << "Invalid input. Order must be at least 3. Exiting." << std::endl;
        return 1;
    }

    // Use compile-time dispatch based on order
    if (n == 3) {
        run_demo<3>();
    } else if (n == 4) {
        run_demo<4>();
    } else if (n == 5) {
        run_demo<5>();
    } else if (n == 6) {
        run_demo<6>();
    } else {
        std::cout << "Order " << n << " not supported in this demo. Please use 3-6." << std::endl;
        return 1;
    }

    std::cout << "\nExiting..." << std::endl;
    return 0;
}
