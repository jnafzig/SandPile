//#include <emscripten.h>
#include <SDL2/SDL.h>
#include "pile.h"

#include <iostream>
#include <chrono>

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
  std::cout <<  "Error at " << str << ": " << SDL_GetError() << std::endl;
    fprintf(stderr, "Error at %s: %s\n", str, SDL_GetError());
  //  emscripten_force_exit(1);
}

void draw(pile &sandpile, int N) {

  SDL_Window *window;
  SDL_Surface *surface;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) sdlError("SDL_Init");

  window = SDL_CreateWindow("Sand Piles",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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


  SDL_SaveBMP(surface, "out.bmp");
  //Wait two seconds
  SDL_Delay( 2000 );

  //Destroy window
  SDL_DestroyWindow( window );
  //Quit SDL subsystems
  SDL_Quit();
}

void makeSymmetricPile(pile &sandpile, int width, long numGrains) {

  int triNum = triangleNum(width);
  int previousTriNum = triangleNum(width-1);
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

  sandpile.nodes[0]->height = numGrains;
}

int main(void) {

  using namespace std::chrono;

  high_resolution_clock::time_point t1 = high_resolution_clock::now();

  int width = 150;
  long numGrains = pow(2,17);
  int triNum = triangleNum(width);
  pile sandpile(triNum);
  makeSymmetricPile(sandpile, width, numGrains);

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

  draw(sandpile, width);

  t2 = high_resolution_clock::now();
  time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "painting done.  Time elapsed: " << time_span.count() << std::endl;


}
