#ifndef PILE_H
#define PILE_H

#include <vector>
#include <mutex>

struct pile;

struct pile {
    std::vector<std::vector<unsigned int>> nodes;
    std::vector<int> j_range;
    int i_range;
    pile(int N);
    void stabilize();
    int worker(std::vector<std::mutex>&);
    bool stabilize_grid(std::vector<std::mutex>&);
    int check_grid(std::vector<std::mutex>&);
};

#endif
