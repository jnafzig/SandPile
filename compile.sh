# em++ -O2 -std=c++11 -s USE_SDL=2 sandpile.cpp -o docs/index.html

g++ -O2 -std=c++11 sandpile.cpp pile.cpp -lSDL2
