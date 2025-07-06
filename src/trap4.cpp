//header includes

#include "trap4.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cmath>
#include <string>
#include <SDL2/SDL_ttf.h>
#include <bits/stdc++.h>
#include "music.h"
using namespace std;
//Constants

const int GRID_W = 10;
const int GRID_H = 10;
const int TILE_SIZE = 80;
const int WINDOW_W = GRID_W * TILE_SIZE, WINDOW_H = GRID_H * TILE_SIZE;

//map info

int map_data[GRID_H][GRID_W] = {
	{0,2,0,0,0,1,0,0,0,2},
	{0,0,1,0,0,0,0,1,2,0},
	{3,0,0,0,1,3,0,0,0,0},
	{1,1,0,2,0,3,0,0,0,1},
	{0,0,2,0,0,2,0,1,0,0},
	{0,3,0,0,3,0,0,0,2,0},
	{0,1,1,0,0,0,2,0,0,1},
	{2,0,0,3,2,0,0,1,0,0},
	{0,0,2,3,0,0,0,2,0,0},
	{0,2,1,0,3,0,1,0,1,0}
};


//collectable gems

bool hasGemInit[GRID_H][GRID_W] = {
	{0,0,0,0,0,1,0,0,0,0},
	{0,0,0,1,0,0,0,0,1,0},
	{0,0,0,0,1,0,0,1,0,0},
	{0,0,1,0,0,0,0,0,1,0},
	{1,0,0,1,0,0,1,0,0,0},
	{0,1,0,0,0,0,0,0,0,0},
	{1,0,1,0,1,0,1,0,0,0},
	{0,0,0,0,0,0,0,0,0,1},
	{0,0,1,0,0,0,0,0,0,0},
	{1,0,0,0,1,0,1,0,0,0}
};


//function for spik frame

int spikeFrame(int type, float elapsedSec) {
	float period = float(type);
	float phase = fmod(elapsedSec, period);
	float segment = period / 3.0f;
	if (phase < segment) return 0;
	else if (phase < 2*segment) return 1;
	else return 2;
}

//spike ball informations

const float SPIKEBALL_SPEED = 300.0f;
const int SPIKEBALL_SIZE = 64;
const float GAP_DURATION = 1.5f;

float col3_y, col6_y, row4_x, row8_x;
int activeGroup = 1;
bool ballsActive = false;
float groupStartTime = 0.0f;
const int colA = 3, colB = 6, rowA = 4, rowB = 8;

SDL_Texture* spikeBallTextures[6];
SDL_Texture* doorTextures[4];
bool doorAnimating = false;
int doorFrame = 0;
float doorTimer = 0.0f;

//function to reset spikeballs once off screen

void resetSpikeBalls() {
	col3_y = WINDOW_H + SPIKEBALL_SIZE;
	col6_y = -SPIKEBALL_SIZE;
	row4_x = WINDOW_W + SPIKEBALL_SIZE;
	row8_x = -SPIKEBALL_SIZE;
	activeGroup = 1;
	groupStartTime = 0.0f;
	ballsActive = true;
}

// collision checker function

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
	return SDL_HasIntersection(&a, &b);
}

//spikeball animation loading texture

bool loadSpikeBallTextures(SDL_Renderer* ren) {
	for (int i = 0; i < 6; i++) {
		string fname = "assets/spikeball" + to_string(i+1) + ".png";
		spikeBallTextures[i] = IMG_LoadTexture(ren, fname.c_str());

	}
	return true;
}

//door animation loading texture

bool loadDoorTextures(SDL_Renderer* ren) {
	for (int i = 0; i < 4; i++) {
		string fname = "assets/door" +to_string(i+1) + ".png";
		doorTextures[i] = IMG_LoadTexture(ren, fname.c_str());

	}
	return true;
}

//game function

bool runTrap4(SDL_Window* window, SDL_Renderer* ren,Uint32 gameStartTime, bool timerRunning) {

	//refresh games on each entry
	bool hasGem[GRID_H][GRID_W];
	for (int y = 0; y < GRID_H; ++y)
		for (int x = 0; x < GRID_W; ++x)
			hasGem[y][x] = hasGemInit[y][x];

	//Loading textures


	SDL_Texture* texTile      = IMG_LoadTexture(ren, "assets/normal.png");
	SDL_Texture* texNoSpike   = IMG_LoadTexture(ren, "assets/nospike.png");
	SDL_Texture* texHalfSpike = IMG_LoadTexture(ren, "assets/halfspike.png");
	SDL_Texture* texFullSpike = IMG_LoadTexture(ren, "assets/fullspike.png");
	SDL_Texture* texGengar    = IMG_LoadTexture(ren, "assets/gengar.png");
	SDL_Texture* texGengarite = IMG_LoadTexture(ren, "assets/gengarite.png");

	//check if animation images are alright

	if (!loadSpikeBallTextures(ren)) {
		return false;
	}
	if (!loadDoorTextures(ren)) {
		return false;
	}

	//necessary constant for time count show and animation

	resetSpikeBalls();
	int gx = 0, gy = 0;
	bool playing = true;
	bool trapFinished = false;
	SDL_Event e;
	Uint32 startTicks = SDL_GetTicks();
	Uint32 lastTicks = startTicks;
	int spikeBallFrame = 0;
	float animationTimer = 0.0f;
	doorAnimating = false;
	doorFrame = 0;
	doorTimer = 0.0f;

	//check if all gems are collected or not

	auto allGemsCollected = [&]() {
		for (int y = 0; y < GRID_H; ++y)
			for (int x = 0; x < GRID_W; ++x)
				if (hasGem[y][x]) return false;
		return true;
	};

	//main event loop
	while (playing) {

		Uint32 nowTicks = SDL_GetTicks();
		float elapsed = (nowTicks - startTicks) / 1000.0f;
		float dt = (nowTicks - lastTicks) / 1000.0f;
		lastTicks = nowTicks;

		// Underfoot spike
		int t = map_data[gy][gx];
		if (t > 0) {
			int frame = spikeFrame(t, elapsed);
			if (frame == 1 || frame == 2) {
				gx = 0;
				gy = 0;
			}
		}
		animationTimer += dt;
		if (animationTimer >= 0.05f) {
			spikeBallFrame = (spikeBallFrame + 1) % 6;
			animationTimer = 0.0f;
		}
		if (doorAnimating) {
			doorTimer += dt;
			if (doorTimer >= 0.2f && doorFrame < 3) {
				doorFrame++;
				doorTimer = 0.0f;
			}
			else if (doorTimer >= 0.2f && doorFrame == 3) {
				doorAnimating = false;
				trapFinished = true;
				break;
			}
		}
		while (SDL_PollEvent(&e)) {

			//quit button handling

			if (e.type == SDL_QUIT) {
				playing = false;
				trapFinished = false;
				break;
			}

			//WASD player movement handling

			else if (e.type == SDL_KEYDOWN) {
				int nx = gx, ny = gy;
				if (e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_UP) {
					ny--;
				}
				else if (e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_DOWN) {
					ny++;
				}
				else if (e.key.keysym.sym == SDLK_a || e.key.keysym.sym == SDLK_LEFT) {
					nx--;
				}
				else if (e.key.keysym.sym == SDLK_d || e.key.keysym.sym == SDLK_RIGHT) {
					nx++;
				}



				//confine player within the window and make the gems vanish if collected

				if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
					gx = nx;
					gy = ny;
					int t = map_data[gy][gx];
					if (t > 0) {
						int frame = spikeFrame(t, elapsed);
						if (frame == 1 || frame == 2) {
							gx = 0;
							gy = 0;
							continue;
						}
					}
					if (hasGem[gy][gx]) hasGem[gy][gx] = false;
				}


				//after collecting all gems , on pressing enter start door animation

				if (e.key.keysym.sym == SDLK_RETURN &&
				        gx == GRID_W-1 && gy == GRID_H-1 &&
				        !doorAnimating && allGemsCollected()) {
					doorAnimating = true;
					doorFrame = 0;
					doorTimer = 0.0f;
				}
			}
		}


		//spike ball movement logic
		if (!ballsActive) {
			float now = SDL_GetTicks() / 1000.0f;
			if (now - groupStartTime >= GAP_DURATION) {
				ballsActive = true;
				groupStartTime = now;
				if (activeGroup == 1) {
					activeGroup = 2;
					row4_x = WINDOW_W + SPIKEBALL_SIZE;
					row8_x = -SPIKEBALL_SIZE;
				}
				else {
					activeGroup = 1;
					col3_y = WINDOW_H + SPIKEBALL_SIZE;
					col6_y = -SPIKEBALL_SIZE;
				}
			}
		} else {
			if (activeGroup == 1) {
				col3_y -= SPIKEBALL_SPEED * dt;
				col6_y += SPIKEBALL_SPEED * dt;
				if (col3_y < -SPIKEBALL_SIZE && col6_y > WINDOW_H + SPIKEBALL_SIZE) {
					ballsActive = false;
					groupStartTime = SDL_GetTicks() / 1000.0f;
				}
			} else {
				row4_x -= SPIKEBALL_SPEED * dt;
				row8_x += SPIKEBALL_SPEED * dt;
				if (row4_x < -SPIKEBALL_SIZE && row8_x > WINDOW_W + SPIKEBALL_SIZE) {
					ballsActive = false;
					groupStartTime = SDL_GetTicks() / 1000.0f;
				}
			}
		}

		SDL_Rect gRect{ gx * TILE_SIZE, gy * TILE_SIZE - 10, TILE_SIZE, TILE_SIZE };

		if (ballsActive) {
			SDL_Rect ballRect{0, 0, SPIKEBALL_SIZE, SPIKEBALL_SIZE};
			if (activeGroup == 1) {
				if (gx == colA) {
					ballRect.x = colA * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
					ballRect.y = (int)col3_y;
					if (checkCollision(gRect, ballRect)) {
						gx = 0;
						gy = 0;
					}
				}
				if (gx == colB) {
					ballRect.x = colB * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
					ballRect.y = (int)col6_y;
					if (checkCollision(gRect, ballRect)) {
						gx = 0;
						gy = 0;
					}
				}
			} else {
				if (gy == rowA) {
					ballRect.x = (int)row4_x;
					ballRect.y = rowA * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
					if (checkCollision(gRect, ballRect)) {
						gx = 0;
						gy = 0;
					}
				}
				if (gy == rowB) {
					ballRect.x = (int)row8_x;
					ballRect.y = rowB * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
					if (checkCollision(gRect, ballRect)) {
						gx = 0;
						gy = 0;
					}
				}
			}
		}

		// clear screen
		SDL_RenderClear(ren);
		//start rendering

		for (int y = 0; y < GRID_H; ++y) for (int x = 0; x < GRID_W; ++x) {

				if (x == GRID_W - 1 && y == GRID_H - 1) continue;
				SDL_Rect tileRect{ x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
				SDL_RenderCopy(ren, texTile, nullptr, &tileRect);
				int t = map_data[y][x];
				if (t > 0) {
					int frame = spikeFrame(t, elapsed);
					SDL_Texture* spikeTex = (frame == 0 ? texNoSpike : frame == 1 ? texHalfSpike : texFullSpike);
					SDL_RenderCopy(ren, spikeTex, nullptr, &tileRect);
				}
			}

		SDL_Rect doorRect{ (GRID_W - 1)*TILE_SIZE, (GRID_H - 1)*TILE_SIZE, TILE_SIZE, TILE_SIZE };
		SDL_Texture* toDraw = doorTextures[0];
		if (doorAnimating) toDraw = doorTextures[doorFrame];
		SDL_RenderCopy(ren, toDraw, nullptr, &doorRect);

		SDL_Rect gdst{ gx * TILE_SIZE, gy * TILE_SIZE - 10, TILE_SIZE, TILE_SIZE };
		SDL_RenderCopy(ren, texGengar, nullptr, &gdst);

		if (ballsActive) {
			SDL_Rect ballRect{ 0, 0, SPIKEBALL_SIZE, SPIKEBALL_SIZE };
			int frameIndex = spikeBallFrame % 6;
			if (activeGroup == 1) {
				ballRect.x = colA * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
				ballRect.y = (int)col3_y;
				SDL_RenderCopy(ren, spikeBallTextures[frameIndex], nullptr, &ballRect);
				ballRect.x = colB * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
				ballRect.y = (int)col6_y;
				SDL_RenderCopy(ren, spikeBallTextures[frameIndex], nullptr, &ballRect);
			}
			else {
				ballRect.x = (int)row4_x;
				ballRect.y = rowA * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
				SDL_RenderCopy(ren, spikeBallTextures[frameIndex], nullptr, &ballRect);
				ballRect.x = (int)row8_x;
				ballRect.y = rowB * TILE_SIZE + (TILE_SIZE - SPIKEBALL_SIZE)/2;
				SDL_RenderCopy(ren, spikeBallTextures[frameIndex], nullptr, &ballRect);
			}
		}
		for (int y = 0; y < GRID_H; ++y)
			for (int x = 0; x < GRID_W; ++x)
				if (hasGem[y][x]) {
					SDL_Rect gemRect;
					gemRect.w = 24;
					gemRect.h = 24;
					gemRect.x = x * TILE_SIZE + (TILE_SIZE - gemRect.w)/2;
					gemRect.y = y * TILE_SIZE + (TILE_SIZE - gemRect.h)/2 ;
					SDL_RenderCopy(ren, texGengarite, nullptr, &gemRect);
				}

		//condition to show time on screen in the trap rooms too.It is common in every trap room and main map

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
			SDL_Texture* timerTex = SDL_CreateTextureFromSurface(ren, surf);
			SDL_Rect timerRect = {20, 20, surf->w, surf->h};

			SDL_RenderCopy(ren, timerTex, nullptr, &timerRect);

			SDL_FreeSurface(surf);
			SDL_DestroyTexture(timerTex);
			TTF_CloseFont(fontTimer);
		}

		SDL_RenderPresent(ren);
	}
	//destroy textures

	SDL_DestroyTexture(texTile);
	SDL_DestroyTexture(texNoSpike);
	SDL_DestroyTexture(texHalfSpike);
	SDL_DestroyTexture(texFullSpike);
	SDL_DestroyTexture(texGengar);
	SDL_DestroyTexture(texGengarite);
	//destroying animation textures
	for (int i = 0; i < 6; i++) SDL_DestroyTexture(spikeBallTextures[i]);
	for (int i = 0; i < 4; i++) SDL_DestroyTexture(doorTextures[i]);
	// Do NOT destroy renderer or window.
	return trapFinished;
}


