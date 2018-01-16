#ifndef DANCINGLINKS_H
#define DANCINGLINKS_H

#include <vector>
#include <set>
#include <cmath>


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
  bool spillChain();
};

struct pile {
  std::vector<node*> nodes;
  node* sinknodePtr;
  pile(int N);
  ~pile();
  void makeLink(int node1, int node2, int linkWeight);
  void stabilizeWithChaining();
  void stabilizeWithSet();
  void stabilize();
};

#endif
