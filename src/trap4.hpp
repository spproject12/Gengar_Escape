#ifndef TRAP4_HPP
#define TRAP4_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Returns true if player finished trap, false if quit
bool runTrap4(SDL_Window* window, SDL_Renderer* renderer,Uint32 gameStartTime, bool timerRunning);

#endif

