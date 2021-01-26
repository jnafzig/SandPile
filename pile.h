#ifndef PILE_H
#define PILE_H

#include <vector>

struct node;
struct pile;

struct link {
  node* linkToNode;
  long linkWeight;
  link(node* nodePtr, int linkWeight);
  long spill(long spillover);
};

struct node {
  long height = 0;
  long heightLimit = 4;
  std::vector<link> links;
  bool spill();
};

struct pile {
  std::vector<std::vector<node*>> nodes;
  pile(int N);
  ~pile();
  void makeLink(int inode1, int jnode1, int inode2, int jnode2, int linkWeight);
  void stabilize();
  void stabilize1();
  void stabilize_stripes();
};

#endif
