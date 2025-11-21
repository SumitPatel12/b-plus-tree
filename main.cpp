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
            std::cout << "Inserting " << key << "..." << std::endl;
            tree.insert(key, nullptr);
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
    std::cout << "Enter B-Tree Order (N) [3-10]: ";
    if (!(std::cin >> n)) {
        std::cout << "Invalid input. Exiting." << std::endl;
        return 1;
    }

    if (n < 3 || n > 10) {
        std::cout << "Order must be between 3 and 10." << std::endl;
        return 1;
    }

    switch (n) {
        case 3: run_demo<3>(); break;
        case 4: run_demo<4>(); break;
        case 5: run_demo<5>(); break;
        case 6: run_demo<6>(); break;
        case 7: run_demo<7>(); break;
        case 8: run_demo<8>(); break;
        case 9: run_demo<9>(); break;
        case 10: run_demo<10>(); break;
        default: std::cout << "Unsupported order." << std::endl; break;
    }

    std::cout << "Exiting..." << std::endl;
    return 0;
}
