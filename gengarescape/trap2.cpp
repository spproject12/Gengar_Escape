// trap2.cpp

#include <iostream>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdlib> 
#include <ctime> 
#include <SDL2/SDL_ttf.h>
#include "music.h"

using namespace std;

struct Bullet 
{
    float x, y;       // Float position
    float vx, vy;     // Velocity in x and y
    SDL_Rect rect;    // For drawing
};

struct Characters
{
    float x, y;
    float vx, vy;
    SDL_Rect charRect;
    int health = 5;
    bool show = true;

    Characters(float x = 0, float y = 0, float vx = 0, float vy = 0, SDL_Rect charRect = {0, 0, 0, 0},
                int health = 5, bool show = true)
    : x(x), y(y), vx(vx), vy(vy), charRect(charRect), health(health), show(show)
    {}
};



int tileSize = 80;
int gridWidth = 10;
int gridHeight = 10;
const int window_width = gridWidth*tileSize;
const int window_height = gridHeight*tileSize;

int roomGrid[10][10] = {
    {0,1,5,2,3,6,2,0,0,0},
    {0,4,0,0,0,0,0,0,1,0},
    {0,5,0,4,0,0,0,0,0,5},
    {0,1,0,0,1,0,0,0,0,2},
    {1,0,0,0,2,2,0,1,0,0},
    {0,0,0,0,0,5,0,4,0,1},
    {0,1,0,0,0,0,0,0,1,0},
    {0,5,1,0,0,0,0,1,5,0},
    {0,4,0,0,0,0,0,0,0,0},
    {5,0,0,0,1,1,0,0,0,0},
};

vector<vector<bool>> crackedTileShow(10, vector<bool>(10, true));
vector<vector<bool>> iceStoneShow(10, vector<bool>(10, true));

// Wrapper function for the trap room, like runTrap2(window, renderer)
bool runTrap2(SDL_Window* window, SDL_Renderer* renderer,Uint32 gameStartTime, bool timerRunning) {
    // Reset all state for a fresh entry each time
    
    int tileSizeLocal = tileSize;
    int gridWidthLocal = gridWidth;
    int gridHeightLocal = gridHeight;
    int roomGridLocal[10][10];
    for (int i = 0; i < 10; ++i) for (int j = 0; j < 10; ++j) roomGridLocal[i][j] = roomGrid[i][j];
    vector<vector<bool>> crackedTileShowLocal = crackedTileShow;
    vector<vector<bool>> iceStoneShowLocal = iceStoneShow;

    Characters gengar(4 * tileSizeLocal, 9 * tileSizeLocal, 0, 0, {4 * tileSizeLocal, 9 * tileSizeLocal, tileSizeLocal, tileSizeLocal}, 5, true);
    int stone = 0;
    const int totalStone = 7; // original logic expects 7 icestones

    SDL_Texture* texGengar = IMG_LoadTexture(renderer, "gengar.png");
    SDL_Texture* texPit = IMG_LoadTexture(renderer, "icepit.png");
    SDL_Texture* texBoulder = IMG_LoadTexture(renderer, "iceboulder.png");
    SDL_Texture* texCrackedTile = IMG_LoadTexture(renderer, "crackedicetile.png");
    SDL_Texture* texIce = IMG_LoadTexture(renderer, "icetile.png");
    SDL_Texture* texDoorLeft = IMG_LoadTexture(renderer, "icedoorleft.png");
    SDL_Texture* texVoidTile = IMG_LoadTexture(renderer, "voidtile.png");
    SDL_Texture* texIceStone = IMG_LoadTexture(renderer, "iceStone.png");
    SDL_Texture* texDoorRight = IMG_LoadTexture(renderer, "icedoorright.png");

    bool gameIsRunning = true;
    bool playerCompleted = false; // set true when player exits correctly

    while (gameIsRunning) 
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) 
            {
                gameIsRunning = false;
            }

            if (event.type == SDL_KEYDOWN) 
            {
                if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) 
                {
                    while (true)
                    {
                        int nextY = gengar.charRect.y / tileSizeLocal - 1;
                        int nextX = gengar.charRect.x / tileSizeLocal;

                        if (roomGridLocal[nextY][nextX] == 1)
                        {
                            gengar.charRect.y -= tileSizeLocal; 
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 2)
                        {
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 3)
                        {
                            if (stone == totalStone) {
                                // Exit trigger: collected all icestones and touched door
                                playerCompleted = true;
                                gameIsRunning = false;
                                break;
                            } else {
                                gengar.charRect.x = 4 * tileSizeLocal;
                                gengar.charRect.y = 9 * tileSizeLocal;
                            }
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 4)
                        {
                            crackedTileShowLocal[nextY][nextX] = false;
                            gengar.charRect.x = 4 * tileSizeLocal;
                            gengar.charRect.y = 9 * tileSizeLocal;
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 5)
                        {
                            iceStoneShowLocal[nextY][nextX] = false;
                            stone++;
                            roomGridLocal[nextY][nextX] = 0;
                        }
                        else if (gengar.charRect.y - tileSizeLocal >= 0)
                        {
                            gengar.charRect.y -= tileSizeLocal; 
                        }
                        else break;
                    }
                }
                
                if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN)
                {
                    while (true)
                    {
                        int nextY = gengar.charRect.y / tileSizeLocal + 1;
                        int nextX = gengar.charRect.x / tileSizeLocal;
                        if (nextY < 0 || nextY >= gridHeight || nextX < 0 || nextX >= gridWidth) {
            break; // Out of bounds, don't move
        }
                        if (roomGridLocal[nextY][nextX] == 1)
                        {
                            gengar.charRect.y += tileSizeLocal; 
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 2)
                        {
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 3)
                        {
                            if (stone == totalStone) {
                                // Exit trigger
                                playerCompleted = true;
                                gameIsRunning = false;
                                break;
                            } else {
                                gengar.charRect.x = 4 * tileSizeLocal;
                                gengar.charRect.y = 9 * tileSizeLocal;
                            }
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 4)
                        {
                            crackedTileShowLocal[nextY][nextX] = false;
                            gengar.charRect.x = 4 * tileSizeLocal;
                            gengar.charRect.y = 9 * tileSizeLocal;
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 5)
                        {
                            iceStoneShowLocal[nextY][nextX] = false;
                            stone++;
                            roomGridLocal[nextY][nextX] = 0;
                        }
                        else if (gengar.charRect.y + tileSizeLocal < 800)
                        {
                            gengar.charRect.y += tileSizeLocal;
                        }
                        else break;
                    }
                }

                if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT)
                {
                    while (true)
                    {
                        int nextY = gengar.charRect.y / tileSizeLocal;
                        int nextX = gengar.charRect.x / tileSizeLocal - 1;
  if (nextY < 0 || nextY >= gridHeight || nextX < 0 || nextX >= gridWidth) {
            break; // Out of bounds, don't move
        }
                        if (roomGridLocal[nextY][nextX] == 1)
                        {
                            gengar.charRect.x -= tileSizeLocal; 
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 2)
                        {
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 3)
                        {
                            if (stone == totalStone) {
                                // Exit trigger
                                playerCompleted = true;
                                gameIsRunning = false;
                                break;
                            } else {
                                gengar.charRect.x = 4 * tileSizeLocal;
                                gengar.charRect.y = 9 * tileSizeLocal;
                            }
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 4)
                        {
                            crackedTileShowLocal[nextY][nextX] = false;
                            gengar.charRect.x = 4 * tileSizeLocal;
                            gengar.charRect.y = 9 * tileSizeLocal;
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 5)
                        {
                            iceStoneShowLocal[nextY][nextX] = false;
                            stone++;
                            roomGridLocal[nextY][nextX] = 0;
                        }
                        else if (gengar.charRect.x - tileSizeLocal >= 0)
                        {
                            gengar.charRect.x -= tileSizeLocal;
                        }
                        else break;
                    }
                }

                if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT)
                {
                    while (true)
                    {
                        int nextY = gengar.charRect.y / tileSizeLocal;
                        int nextX = gengar.charRect.x / tileSizeLocal + 1;

                        if (roomGridLocal[nextY][nextX] == 1)
                        {
                            gengar.charRect.x += tileSizeLocal; 
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 2)
                        {
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 3)
                        {
                            if (stone == totalStone) {
                                // Exit trigger
                                playerCompleted = true;
                                gameIsRunning = false;
                                break;
                            } else {
                                gengar.charRect.x = 4 * tileSizeLocal;
                                gengar.charRect.y = 9 * tileSizeLocal;
                            }
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 4)
                        {
                            crackedTileShowLocal[nextY][nextX] = false;
                            gengar.charRect.x = 4 * tileSizeLocal;
                            gengar.charRect.y = 9 * tileSizeLocal;
                            break;
                        }
                        else if (roomGridLocal[nextY][nextX] == 5)
                        {
                            iceStoneShowLocal[nextY][nextX] = false;
                            stone++;
                            roomGridLocal[nextY][nextX] = 0;
                        }
                        else if (gengar.charRect.x + tileSizeLocal < 800)
                        {
                            gengar.charRect.x += tileSizeLocal;
                        }
                        else break;
                    }
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for (int x = 0; x < gridWidthLocal; x++)
        {
            for (int y = 0; y < gridHeightLocal; y++) 
            {
                SDL_Rect tileRect{x * tileSizeLocal, y * tileSizeLocal, tileSizeLocal, tileSizeLocal};

                if (roomGridLocal[y][x] == 0)
                SDL_RenderCopy(renderer, texIce, NULL, &tileRect);

                else if (roomGridLocal[y][x] == 1)
                SDL_RenderCopy(renderer, texPit, NULL, &tileRect);

                else if (roomGridLocal[y][x] == 2)
                SDL_RenderCopy(renderer, texBoulder, NULL, &tileRect);

                else if (roomGridLocal[y][x] == 3)
                SDL_RenderCopy(renderer, texDoorLeft, NULL, &tileRect);

                else if (roomGridLocal[y][x] == 6)
                SDL_RenderCopy(renderer, texDoorRight, NULL, &tileRect);

                else if (roomGridLocal[y][x] == 4 && crackedTileShowLocal[y][x] == true)
                SDL_RenderCopy(renderer, texCrackedTile, NULL, &tileRect);
                
                else if (roomGridLocal[y][x] == 4 && crackedTileShowLocal[y][x] == false)
                SDL_RenderCopy(renderer, texVoidTile, NULL, &tileRect);

                else if (roomGridLocal[y][x] == 5 && iceStoneShowLocal[y][x] == true)
                {
                    SDL_RenderCopy(renderer, texIce, NULL, &tileRect);
                    SDL_RenderCopy(renderer, texIceStone, NULL, &tileRect);
                }
                
                else if (roomGridLocal[y][x] == 5 && iceStoneShowLocal[y][x] == false)
                {
                    SDL_RenderCopy(renderer, texIce, NULL, &tileRect);
                }
            }
        }

         if (gengar.show == true)
           SDL_RenderCopy(renderer, texGengar, NULL, &gengar.charRect);
         
          
      // ... trap room game loop ...
if (timerRunning) {
    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - gameStartTime;
    int minutes = elapsed / 60000;
    int seconds = (elapsed % 60000) / 1000;

    char timerText[16];
    snprintf(timerText, sizeof(timerText), "%02d:%02d", minutes, seconds);

    SDL_Color color = {255, 255, 255}; // White
   TTF_Font* fontTimer = TTF_OpenFont("fonthealth.ttf", 36);
    SDL_Surface* surf = TTF_RenderText_Solid(fontTimer, timerText, color);
    SDL_Texture* timerTex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect timerRect = {20, 20, surf->w, surf->h};

    SDL_RenderCopy(renderer, timerTex, nullptr, &timerRect);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(timerTex);
    TTF_CloseFont(fontTimer);
}

        SDL_RenderPresent(renderer);
    }

    // Cleanup all textures
    SDL_DestroyTexture(texGengar);
    SDL_DestroyTexture(texPit);
    SDL_DestroyTexture(texBoulder);
    SDL_DestroyTexture(texCrackedTile);
    SDL_DestroyTexture(texIce);
    SDL_DestroyTexture(texDoorLeft);
    SDL_DestroyTexture(texDoorRight);
    SDL_DestroyTexture(texVoidTile);
    SDL_DestroyTexture(texIceStone);

    // Return true if player finished, false if quit
    return playerCompleted;
}


