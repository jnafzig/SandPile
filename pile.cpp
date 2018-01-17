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

void pile::stabilizeWithChaining() {
  bool done = false;
  int spillover;
  while (!done) {
    done = true;
    for (auto &nodeVector : nodes) {
      for (auto nodePtr : nodeVector) {
        if (nodePtr->height >= nodePtr->heightLimit) {
          done = false;
          do {
            spillover = nodePtr->height / nodePtr->heightLimit;
            nodePtr->height = nodePtr->height % nodePtr->heightLimit;
            for (auto &link : nodePtr->links) {
              if (link.spill(spillover) > nodePtr->height) nodePtr = link.linkToNode;
            }
          } while (nodePtr->height >= nodePtr->heightLimit);
        }
      }
    }
  }
}
