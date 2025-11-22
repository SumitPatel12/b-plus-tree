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
    std::cout << "  r <min> <max> - Find keys in range [min, max)" << std::endl;
    std::cout << "  q       - Quit" << std::endl;
    std::cout << "========================================" << std::endl;

    std::string command;
    int key;
    int range_min, range_max;
    
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
            
        } else if (command == "r" || command == "range") {
             if (!(std::cin >> range_min >> range_max)) {
                std::cout << "Invalid range. Please enter two integers." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            std::vector<int> keys = tree.find_keys_in_range(range_min, range_max);
            std::cout << "Keys in range [" << range_min << ", " << range_max << "): ";
            if (keys.empty()) {
                std::cout << "None";
            } else {
                for (size_t i = 0; i < keys.size(); ++i) {
                    std::cout << keys[i] << (i < keys.size() - 1 ? ", " : "");
                }
            }
            std::cout << std::endl;

            std::cout << "\nCurrent Tree State:" << std::endl;
            tree.print();

        } else {
            std::cout << "Unknown command. Use 'i <key>' to insert, 'd <key>' to delete, 'r <min> <max>' to find range, or 'q' to quit." << std::endl;
        }
    }
}

// Template metaprogramming to support dynamic order selection at runtime
// We instantiate run_demo for a range of N values.
template <std::size_t N>
struct DemoRunner {
    static void run(int n) {
        if (n == N) {
            run_demo<N>();
        } else {
            DemoRunner<N + 1>::run(n);
        }
    }
};

// Base case for recursion (stop at a reasonable max limit, e.g., 20)
template <>
struct DemoRunner<21> {
    static void run(int n) {
        std::cout << "Order " << n << " is too large for this demo (max 20)." << std::endl;
    }
};

int main() {
    std::cout << "Enter B+ Tree Order (N): ";
    int n;
    if (!(std::cin >> n) || n < 3) {
        std::cout << "Invalid input. Order must be at least 3. Exiting." << std::endl;
        return 1;
    }

    // Dispatch to the appropriate template instantiation
    DemoRunner<3>::run(n);

    std::cout << "\nExiting..." << std::endl;
    return 0;
}
