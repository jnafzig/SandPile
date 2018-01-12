#include <SDL.h>
#include <emscripten.h>
#include <vector>
#include <set>
#include <cmath>

#include <iostream>
#include <chrono>

struct node;
struct connection;
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
    //std::cout << spillover << " ";
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
          //std::cout << spillover << " ";
          nodePtr->height = nodePtr->height % nodePtr->heightLimit;
          for (auto &x : nodePtr->links) {
            if (x.spill(spillover) > nodePtr->height) nodePtr = x.linkToNode;
          }
        } while (nodePtr->height >= nodePtr->heightLimit);
      }
    }
  }
}

int colIndex(int index) {
  return ((int) sqrt(8 * index + 1) -1) / 2;
}

int triangleNum(int n) {
  return n * (n + 1) / 2;
}

int lowerBoundTriangleNum(int index) {
  return triangleNum(colIndex(index));
}

int rightIndex(int index) {
  return index + colIndex(index) + 1;
}

int rightWeight(int index) {
  return 1;
}

int leftIndex(int index) {
  if (index + 1 == lowerBoundTriangleNum(index + 1)) return -1;
  return index - colIndex(index);
}

int leftWeight(int index) {
  if (index == 1) return 4;
  if (index + 2 == lowerBoundTriangleNum(index + 2)) return 2;
  return 1;
}

int upIndex(int index) {
  if (index + 1 == lowerBoundTriangleNum(index + 1)) return -1;
  return index + 1;
}

int upWeight(int index) {
  if (index + 2 == lowerBoundTriangleNum(index + 2)) return 2;
  return 1;
}

int downIndex(int index) {
  if (index == lowerBoundTriangleNum(index)) return -1;
  return index - 1;
}

int downWeight(int index) {
  if (index - 1 == lowerBoundTriangleNum(index - 1)) return 2;
  return 1;
}

void printPile(pile &sandpile) {
  for (int lb = 0; lb < colIndex(sandpile.nodes.size()); lb++) {
    for (int i = 0; i <= lb; i++) {
      std::cout << sandpile.nodes[triangleNum(lb) + i]->height;
    }
    std::cout << std::endl;
  }

}


static void sdlError(const char *str)
{
    fprintf(stderr, "Error at %s: %s\n", str, SDL_GetError());
    emscripten_force_exit(1);
}

void draw(pile &sandpile, int N) {

  SDL_Window *window;
  SDL_Surface *surface;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) sdlError("SDL_Init");

  window = SDL_CreateWindow("SDL 2 test",
                            0, 0,
                            2 * N - 1, 2 * N - 1,
                            SDL_WINDOW_SHOWN);
  if (window == NULL) sdlError("SDL_CreateWindow");
  surface = SDL_GetWindowSurface(window);
  if (surface == NULL) sdlError("SDL_GetWindowSurface");

    int x, y, z;

    if (SDL_MUSTLOCK(surface)) {
        if (SDL_LockSurface(surface) != 0) sdlError("SDL_LockSurface");
    }

    for (y = -N + 1; y < N; y++) {
        Uint32 *p = (Uint32 *)(((Uint8 *)surface->pixels) +
                               surface->pitch * (y+N-1));
        for (x = -N + 1; x < N; x++) {
          if (abs(x) <= abs(y)) {
            z = sandpile.nodes[triangleNum(abs(y)) + abs(x)]->height;
          } else {
            z = sandpile.nodes[triangleNum(abs(x)) + abs(y)]->height;
          }

          switch(z) {
            case 0:
              *(p++) = SDL_MapRGB(surface->format, 255, 255, 255);
              break;
            case 1:
              *(p++) = SDL_MapRGB(surface->format, 0, 150, 230);
              break;
            case 2:
              *(p++) = SDL_MapRGB(surface->format, 250, 200, 0);
              break;
            case 3:
              *(p++) = SDL_MapRGB(surface->format, 178, 40, 100);
              break;
            default:
              std::cout << "missed values " << z << " at x: " << x << " and y: " << y << std::endl;
          }
        }
    }


    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    if (SDL_UpdateWindowSurface(window) != 0)
        sdlError("SDL_UpdateWindowSurface");
}



int main(void) {

  using namespace std::chrono;


  high_resolution_clock::time_point t1 = high_resolution_clock::now();

  int N = 150;
  int triNum = triangleNum(N);
  int previousTriNum = triangleNum(N-1);
  pile sandpile(triNum);
  for (int i = 0; i < previousTriNum; i++) {
    if (upIndex(i) >= 0) sandpile.makeLink(i, upIndex(i), upWeight(i));
    if (leftIndex(i) >= 0) sandpile.makeLink(i, leftIndex(i), leftWeight(i));
    if (downIndex(i) >= 0) sandpile.makeLink(i, downIndex(i), downWeight(i));
    if (rightIndex(i) >= 0) sandpile.makeLink(i, rightIndex(i), rightWeight(i));
  }
  for (int i = previousTriNum; i < triNum; i++) {
    if (upIndex(i) >= 0) sandpile.makeLink(i, upIndex(i), upWeight(i));
    if (leftIndex(i) >= 0) sandpile.makeLink(i, leftIndex(i), leftWeight(i));
    if (downIndex(i) >= 0) sandpile.makeLink(i, downIndex(i), downWeight(i));
    sandpile.makeLink(i, -1, 4); // send right link into sink
  }

  sandpile.nodes[0]->height = (long) pow(2,17);

  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "initialization done.  Time elapsed: " << time_span.count() << std::endl;

  std::cout << sandpile.nodes[0]->height << " grains of sand" << std::endl;
  t1 = high_resolution_clock::now();

  sandpile.stabilizeWithChaining();

  t2 = high_resolution_clock::now();
  time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "stabilization done.  Time elapsed: " << time_span.count() << std::endl;

/*
  t1 = high_resolution_clock::now();
  printPile(sandpile);
  t2 = high_resolution_clock::now();
  time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "printing done.  Time elapsed: " << time_span.count() << std::endl;
*/


  t1 = high_resolution_clock::now();

  draw(sandpile, N);

  t2 = high_resolution_clock::now();
  time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "painting done.  Time elapsed: " << time_span.count() << std::endl;


}
