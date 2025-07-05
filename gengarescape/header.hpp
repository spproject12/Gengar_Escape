#ifndef MAIN_GAME_HPP
#define MAIN_GAME_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

struct GameResult {
    bool won;
    int winTimeSeconds;
};
// Declare the runGame function
extern std::string playerName;  // Declare playerName as an external variable
void renderBackButton();  // Declare renderBackButton function

// Return true if player wins, false otherwise, and return the win time (in seconds) via pointer
GameResult runGame(SDL_Window* window, SDL_Renderer* renderer);



// Any other game-related declarations can go here (such as structs, global variables, etc.)

#endif // MAIN_GAME_HPP

