#include "pile.h"
#include "barrier.h"
#include <thread>
#include <iostream>

long link::spill(long spillover) {
  return linkToNode->height += spillover * linkWeight;
}

link::link(node* nodePtr, int weight) {
  linkToNode = nodePtr;
  linkWeight =  weight;
}

bool node::spill() {
  if (height < heightLimit) return false;
  int spillover = height / heightLimit;
  height = height % heightLimit;
  for (auto &x : links) x.spill(spillover);
  return true;
}

void pile::makeLink(int inode1, int jnode1, int inode2, int jnode2, int linkWeight) {
  nodes[inode1][jnode1]->links.push_back(link(nodes[inode2][jnode2],linkWeight));
}

pile::pile(int N) {
  nodes.resize(N);
  for (int i = 0; i < nodes.size(); i++) {
    nodes[i].resize(i+1);
    for (int j = 0; j < nodes[i].size(); j++) {
      nodes[i][j] = new node;
    }
  }

  // right links
  for (int i = 0; i < N - 1; i++) {
    for (int j = 0; j <= i; j++) {
      makeLink(i, j, i + 1, j, 1);
    }
  }

  // left links
  for (int i = 1; i < N - 1; i++) {
    for (int j = 0; j < i; j++) {
      if (i == 1 & j == 0) {
        makeLink(i, j, i - 1, j, 4);
      } else if (j == i - 1) {
        makeLink(i, j, i - 1, j, 2);
      } else {
        makeLink(i, j, i - 1, j, 1);
      }
    }
  }

  // up links
  for (int i = 1; i < N - 1; i++) {
    for (int j = 0; j < i; j++) {
      if (j == i - 1) {
        makeLink(i, j, i, j + 1, 2);
      } else {
        makeLink(i, j, i, j + 1, 1);
      }
    }
  }

  // down links
  for (int i = 1; i < N - 1; i++) {
    for (int j = 1; j <= i; j++) {
      if (j == 1) {
        makeLink(i, j, i, j - 1, 2);
      } else {
        makeLink(i, j, i, j - 1, 1);
      }
    }
  }

}

pile::~pile() {
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].size(); j++) {
      delete nodes[i][j];
    }
  }
}

void pile::stabilize() {
  bool done = false;
  while (!done) {
    done = true;
    for (auto &nodeVector : nodes) {
      for (auto &nodePtr : nodeVector) {
        done &= !nodePtr->spill();
      }
    }
  }
}


bool stabilizenodes(std::vector<node*> const &node_vector) {
    bool done = true;
    for (auto &nodePtr : node_vector) {
      if (nodePtr->height >= nodePtr->heightLimit) { 
        done = false;
        int spillover = nodePtr->height / nodePtr->heightLimit;
        nodePtr->height = nodePtr->height % nodePtr->heightLimit;
        for (auto &x : nodePtr->links) {
          x.linkToNode->height += spillover * x.linkWeight;
        }
      }
    }
    return done;
}

void stabilizer(Barrier &bar, std::vector<node*> const &nodes_even, std::vector<node*> const &nodes_odd) {
  bool done = false;
  int count = 0;
  while (!done) {
    count += 1;
    done = true;
    done &= stabilizenodes(nodes_even);
    bar.wait();
    done &= stabilizenodes(nodes_odd);
    done = bar.wait_and_check(done);
  }
}

void pile::stabilize1() {
  std::vector<std::vector<node*>> node_groups(4);
  int group_size = nodes.size() / 4;
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].size(); j++) {
      if (i < group_size) {
        node_groups[0].push_back(nodes[i][j]);
      } else if (i < 2*group_size){
        node_groups[1].push_back(nodes[i][j]);
      } else if (i < 3*group_size){
        node_groups[2].push_back(nodes[i][j]);
      } else {
        node_groups[3].push_back(nodes[i][j]);
      }
    }
  }
  Barrier bar(2);
  std::thread worker1(stabilizer, std::ref(bar), std::ref(node_groups[0]), std::ref(node_groups[1]));
  std::thread worker2(stabilizer, std::ref(bar), std::ref(node_groups[2]), std::ref(node_groups[3]));
  worker1.join();
  worker2.join();
}

void pile::stabilize_stripes() {
  bool horizontal = false;
  int num_threads = 7;
  int strip_width = 12;
  std::vector<std::vector<node*>> node_groups(2*num_threads);
  int group_id;
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].size(); j++) {
      if (horizontal) {
        group_id = (j % (2*num_threads*strip_width)) / strip_width;
      } else {
        group_id = (i % (2*num_threads*strip_width)) / strip_width;
      }
      node_groups[group_id].push_back(nodes[i][j]);
    }
  }
  Barrier bar(num_threads);
  std::vector<std::thread> threads;
  for (int i=0; i<2*num_threads; i+=2) {
    threads.push_back(std::thread(stabilizer,
                                  std::ref(bar),
                                  std::ref(node_groups[i]),
                                  std::ref(node_groups[i+1])));
  }
  for (auto &thread : threads) {
    thread.join();
  }
}


