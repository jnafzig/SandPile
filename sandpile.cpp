//#include <emscripten.h>
#include <SDL2/SDL.h>
#include "pile.h"

#include <iostream>
#include <chrono>


void printPile(pile &sandpile) {
  for (int i = 0; i < sandpile.nodes.size(); i++) {
    for (int j = 0; j <= i; j++) {
      std::cout << sandpile.nodes[i][j]->height;
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
          z = sandpile.nodes[abs(y)][abs(x)]->height;
        } else {
          z = sandpile.nodes[abs(x)][abs(y)]->height;
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


int main(void) {

  using namespace std::chrono;

  high_resolution_clock::time_point t1 = high_resolution_clock::now();

  int width = 200;
  long numGrains = pow(2,18);
  pile sandpile(width);
  sandpile.nodes[0][0]->height = numGrains;

  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "initialization done.  Time elapsed: " << time_span.count() << std::endl;


  std::cout << sandpile.nodes[0][0]->height << " grains of sand" << std::endl;
  t1 = high_resolution_clock::now();

  sandpile.stabilize();
  //sandpile.stabilizeWithChaining();

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
