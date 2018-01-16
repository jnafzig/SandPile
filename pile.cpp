#include "pile.h"

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

bool node::spillChain() {
  if (height < heightLimit) return false;
  node* nodePtr = this;
  int spillover;
  do {
    spillover = nodePtr->height / nodePtr->heightLimit;
    nodePtr->height = nodePtr->height % nodePtr->heightLimit;
    for (auto &x : nodePtr->links) {
      if (x.spill(spillover) > nodePtr->height) nodePtr = x.linkToNode;
    }
  } while (nodePtr->height >= nodePtr->heightLimit);
  return true;
}

void pile::makeLink(int node1, int node2, int linkWeight) {
  if (node2 < 0) {
    nodes[node1]->links.push_back(link(sinknodePtr,linkWeight));
  } else {
    nodes[node1]->links.push_back(link(nodes[node2],linkWeight));
  }
}

pile::pile(int N) : nodes(N) {
  for (auto &nodePtr : nodes) {
    nodePtr = new node;
  }
  sinknodePtr = new node;
}

pile::~pile() {
  for (auto &nodePtr : nodes) {
    delete nodePtr;
  }
  delete sinknodePtr;
}

void pile::stabilize() {
  bool done = false;
  while (!done) {
    done = true;
    for (auto &x : nodes) {
      done &= !x->spill();
    }
  }
}

void pile::stabilizeWithSet() {
  std::set<node*> toTopple;
  node* nodePtr;
  int spillover;
  toTopple.insert(nodes[0]);
  while (!toTopple.empty()) {
    nodePtr = (*toTopple.begin());
    toTopple.erase(toTopple.begin());
    spillover = nodePtr->height / nodePtr->heightLimit;
    nodePtr->height = nodePtr->height % nodePtr->heightLimit;
    for (auto &x : nodePtr->links) {
      if (x.spill(spillover) >= nodePtr->heightLimit) {
        toTopple.insert(x.linkToNode);
      }
    }
  }
}

void pile::stabilizeWithChaining() {
  bool done = false;
  int spillover;
  while (!done) {
    done = true;
    for (auto nodePtr : nodes) {
      if (nodePtr->height >= nodePtr->heightLimit) {
        done = false;
        do {
          spillover = nodePtr->height / nodePtr->heightLimit;
          nodePtr->height = nodePtr->height % nodePtr->heightLimit;
          for (auto &x : nodePtr->links) {
            if (x.spill(spillover) > nodePtr->height) nodePtr = x.linkToNode;
          }
        } while (nodePtr->height >= nodePtr->heightLimit);
      }
    }
  }
}
