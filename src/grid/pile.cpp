#include "pile.h"
#include <iostream>
#include <future>
#include <mutex>
#include <chrono>


pile::pile(int N) {
    N = std::max(N, 2);
    nodes.resize(N);
    j_range.resize(N);
    i_range = N-1;
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i].resize(N);
        j_range[i] = 2;
        for (int j = 0; j < nodes[i].size(); j++) {
            nodes[i][j] = 0;
        }
    }
}


bool pile::stabilize_grid(std::vector<std::mutex> &column_guard) {
    bool done = true;
    unsigned int spillover;
    // i = 0 column
    int i = 0;
    int j = 0;
    column_guard[0].lock();
    column_guard[1].lock();
    if (nodes[i][j] >= 4) {
        done = false;
        spillover = nodes[i][j] / 4;
        nodes[i][j] = nodes[i][j] % 4;
        // spills
        nodes[i+1][j] += spillover;
    }
    j = 1;
    if (nodes[i][j] >= 4) {
        done = false;
        spillover = nodes[i][j] / 4;
        nodes[i][j] = nodes[i][j] % 4;
        // spills
        nodes[i+1][j] += spillover;
        nodes[i+1][j-1] += 2*spillover;
    }
    for (j = 2; j < j_range[i]; j++) {
        if (nodes[i][j] >= 4) {
            done = false;
            spillover = nodes[i][j] / 4;
            nodes[i][j] = nodes[i][j] % 4;
            // spills
            nodes[i+1][j-1] += spillover;
            nodes[i+1][j] += spillover;
        }
    }
    // check to expand j_range
    if (nodes[i][j_range[i]] >= 4 and j_range[i] < nodes[i].size() - 1) {
        j = j_range[i]++;
        done = false;
        spillover = nodes[i][j] / 4;
        nodes[i][j] = nodes[i][j] % 4;
        // spills
        nodes[i+1][j-1] += spillover;
        nodes[i+1][j] += spillover;
    }
    // i = 1 column
    i = 1;
    j = 0;
    column_guard[2].lock();
    if (nodes[i][j] >= 4) {
        done = false;
        spillover = nodes[i][j] / 4;
        nodes[i][j] = nodes[i][j] % 4;
        // spills
        nodes[i+1][j] += spillover;
        nodes[i-1][j] += 4*spillover;
        nodes[i-1][j+1] += 2*spillover;
    }
    j = 1;
    if (nodes[i][j] >= 4) {
        done = false;
        spillover = nodes[i][j] / 4;
        nodes[i][j] = nodes[i][j] % 4;
        // spills
        nodes[i+1][j] += spillover;
        nodes[i-1][j] += 2*spillover;
        nodes[i-1][j+1] += 2*spillover;
        nodes[i+1][j-1] += 2*spillover;
    }
    for (j = 2; j < j_range[i]; j++) {
        if (nodes[i][j] >= 4) {
            done = false;
            spillover = nodes[i][j] / 4;
            nodes[i][j] = nodes[i][j] % 4;
            // spills
            nodes[i+1][j-1] += spillover;
            nodes[i+1][j] += spillover;
            nodes[i-1][j+1] += 2*spillover;
            nodes[i-1][j] += 2*spillover;
        }
    }
    // check to expand j_range
    if (nodes[i][j_range[i]] >= 4 and j_range[i] < nodes[i].size() - 1) {
        j = j_range[i]++;
        done = false;
        spillover = nodes[i][j] / 4;
        nodes[i][j] = nodes[i][j] % 4;
        // spills
        nodes[i+1][j-1] += spillover;
        nodes[i+1][j] += spillover;
        nodes[i-1][j+1] += 2*spillover;
        nodes[i-1][j] += 2*spillover;
    }
    // bulk
    for (i = 2; i < i_range; i++) {
        // bottom edge
        j = 0;
        column_guard[i-2].unlock();
        if (i+1 < i_range) {
            column_guard[i+1].lock();
        }
        if (nodes[i][j] >= 4) {
            done = false;
            spillover = nodes[i][j] / 4;
            nodes[i][j] = nodes[i][j] % 4;
            // spills
            nodes[i-1][j+1] += spillover;
            nodes[i+1][j] += spillover;
            nodes[i-1][j] += spillover;
        }
        j = 1;
        if (nodes[i][j] >= 4) {
            done = false;
            spillover = nodes[i][j] / 4;
            nodes[i][j] = nodes[i][j] % 4;
            // spills
            nodes[i-1][j+1] += spillover;
            nodes[i+1][j-1] += 2*spillover;
            nodes[i+1][j] += spillover;
            nodes[i-1][j] += spillover;
        }
        for (j = 2; j < j_range[i]; j++) {
            if (nodes[i][j] >= 4) {
                done = false;
                spillover = nodes[i][j] / 4;
                nodes[i][j] = nodes[i][j] % 4;
                // spills
                nodes[i-1][j+1] += spillover;
                nodes[i+1][j-1] += spillover;
                nodes[i+1][j] += spillover;
                nodes[i-1][j] += spillover;
            }
        }
        // check to expand j_range
        if (nodes[i][j_range[i]] >= 4 and j_range[i] < nodes[i].size() - 1) {
            j = j_range[i]++;
            done = false;
            spillover = nodes[i][j] / 4;
            nodes[i][j] = nodes[i][j] % 4;
            // spills
            nodes[i-1][j+1] += spillover;
            nodes[i+1][j-1] += spillover;
            nodes[i+1][j] += spillover;
            nodes[i-1][j] += spillover;
        }
    }
    column_guard[i_range-2].unlock();
    column_guard[i_range-1].unlock();
    return done;
}

int pile::worker(std::vector<std::mutex> &column_guard) {
    bool done = false;
    int count = 0;
    while (not done) {
        done = stabilize_grid(std::ref(column_guard));
        count++;
    }
    return count;
}

int pile::check_grid(std::vector<std::mutex> &column_guard) {
    int max_height = 0;
    int n_squares = 0;
    column_guard[0].lock();
    for (int i = 0; i < nodes.size(); i++) {
        int max_j = 0;
        for (int j = 0; j < nodes[i].size(); j++) {
            if (nodes[i][j] > max_height) {
                max_height = nodes[i][j];
            }
            if (nodes[i][j] > 0) {
                max_j = j;
            }
        }
        n_squares += max_j;
        if (i+1 < column_guard.size()) {
            column_guard[i+1].lock();
        }
        column_guard[i].unlock();
    }
    std::cout << "num squares " << n_squares << std::endl;
    return max_height;
}

void pile::stabilize() {
    int num_threads(4);
    std::vector<std::future<int>> futures;
    std::vector<std::mutex> column_guard(nodes.size());
    for (int i = 0; i < num_threads; i++) {
        futures.push_back(std::async(&pile::worker, this, 
                          std::ref(column_guard)));
    }

    std::future_status status;
    do {
        status = futures[0].wait_for(std::chrono::milliseconds(200));
        if (status == std::future_status::timeout) {
            std::cout << check_grid(std::ref(column_guard)) << std::endl;
        } else if (status == std::future_status::ready) {
            std::cout << "ready!\n";
        }
    } while (status != std::future_status::ready); 

    int num_iterations = 0;
    for (auto &future: futures) {
        num_iterations += future.get();
    }
    std::cout << num_iterations << " total iterations" << std::endl;
}

