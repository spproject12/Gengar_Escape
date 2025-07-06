//g++ -std=c++17 music.cpp main_map.cpp ui_main.cpp trap1.cpp trap2.cpp trap3.cpp trap4.cpp -o prog -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -ldl

//Header file includes

#include<bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include<SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "trap4.hpp"
#include "trap3.hpp"
#include "trap2.hpp"
#include "trap1.hpp"
#include "header.hpp"
#include "music.h"


using namespace std;

//constants


const int window_height = 800;
const int window_width = 800;

float TileSize = 80;
int GridWidth = 23;
int GridHeight = 23;

const int map_width = 23 * TileSize;
const int map_height = 23 * TileSize;

bool isGameOver = false;
bool hasReloader = false;
bool isWin = false;
bool timerRunning = false;

int totalHits=0;
Uint32 gameStartTime = 0;
Uint32 gameEndTime = 0;

Uint32 lastFireballTime = 0;  // Keeps track of the last emission time
int fireballEmissionInterval = 3000;  // 3 seconds interval for fireball emission

//main map

int RoomGrid[23][23] =
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,3,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,1,2,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,0,0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,1},
	{1,0,0,0,0,1,4,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

vector<SDL_FRect> walls;

//structures
struct Bullet
{
	float x, y;       // Float position
	float vx, vy;     // Velocity in x and y
	SDL_FRect rect;    // For drawing
	bool active; //to check if it is on the screen
};

struct BossFireball {
	float x, y;    // Fireball position
	float vx, vy;  // Fireball velocity
	SDL_FRect rect;  // For rendering
	bool active;   // To check if the fireball is still active
};


struct FloatPoint {
	float x;
	float y;
};

struct Characters
{
	float x, y;
	float vx, vy;
	SDL_FRect charRect;
	int health = 5;
	bool show = true;

	Characters(float x = 0, float y = 0, float vx = 0, float vy = 0, SDL_FRect charRect = {0, 0, 0, 0},
	           int health = 5, bool show = true)
		: x(x), y(y), vx(vx), vy(vy), charRect(charRect), health(health), show(show)
	{}
};

struct Boss {
	float x, y;         // Top-left of boss sprite (in pixels)
	float vx, vy;       // Velocity per frame
	SDL_FRect rect;     // For rendering/collision
	bool active;
	int health=30;
};

struct PortalDef {
	int row, col;
	int nextRow, nextCol;
	bool (*trapFunc)(SDL_Window*, SDL_Renderer*, Uint32 gameStartTime, bool timerRunning);
};

vector<PortalDef> portalDefs = {
	{4, 7,   4, 8,  runTrap1},   // 1st portal: right
	{18, 4,  19, 4, runTrap2},   // 2nd portal: down
	{16, 21, 15, 21, runTrap3},// 3rd portal: up (TRAP3)
	{11, 12, 10, 12, runTrap4} // 4th portal: up (TRAP4)
};



//functions


bool collisionWithWall(SDL_FRect &rect)
{
	for (int i = 0; i < walls.size(); i++)
	{
		if (SDL_HasIntersectionF(&walls[i], &rect) == true)
		{
			return true;
		}
	}

	return false;
}

void respawn(Characters &rect, float x, float y)
{
	rect.health--;
	if (rect.health <= 0) {
		rect.show = false;         // Hide Gengar
		rect.charRect.x = -100;    // Move Gengar off-screen
		rect.charRect.y = -100; // Move Gengar off-screen
		isGameOver = true;
	} else {
		rect.charRect.x = x * TileSize;
		rect.charRect.y = y * TileSize;
	}
}

vector<BossFireball> willowisp;
GameResult runGame(SDL_Window* window, SDL_Renderer* renderer) {

	//Resetting definitions
	playMusic("ui_music.mp3");

	isGameOver = false;
	hasReloader = false;
	isWin = false;
	walls.clear();
	totalHits = 0;
	SDL_SetWindowSize(window, 800, 800);

	//Resetting map

	int newRoomGrid[23][23] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,3,1},
		{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,1,2,1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,0,0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,1},
		{1,0,0,0,0,1,4,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
	};
	for(int i=0; i<23; i++) for(int j=0; j<23; j++) RoomGrid[i][j] = newRoomGrid[i][j];

	gameStartTime = SDL_GetTicks();
	gameEndTime = 0;
	timerRunning = true;

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	SDL_FRect camera = {0, 0, 800, 800};

	//All picture loaded into textures
	SDL_Texture* texture = IMG_LoadTexture(renderer, "assets/gengar.png");
	SDL_Texture* shadow = IMG_LoadTexture(renderer, "assets/bullet.png");
	SDL_Texture* texTile = IMG_LoadTexture(renderer, "assets/tile.png");
	SDL_Texture* texDarkrai = IMG_LoadTexture(renderer, "assets/darkrai.png");
	SDL_Texture* texAlakazam = IMG_LoadTexture(renderer, "assets/mega_alakazam.png");
	SDL_Texture* texPsyball = IMG_LoadTexture(renderer, "assets/psyball.png");
	SDL_Texture* texMimi = IMG_LoadTexture(renderer, "assets/mimi.png");
	SDL_Texture* texIce = IMG_LoadTexture(renderer, "assets/ice.png");
	SDL_Texture* texWall = IMG_LoadTexture(renderer, "assets/wall.png");
	SDL_Texture* texPortal = IMG_LoadTexture(renderer, "assets/portal.png");
	SDL_Texture* texSpirit = IMG_LoadTexture(renderer, "assets/spiritomb.png");
	SDL_Texture* texElecBall = IMG_LoadTexture(renderer, "assets/elecBall.png");
	SDL_Texture* texZekrom = IMG_LoadTexture(renderer, "assets/zekrom.png");
	SDL_Texture* bossTexture = IMG_LoadTexture(renderer, "assets/boss.png");
	SDL_Texture* texWillowisp = IMG_LoadTexture(renderer, "assets/willowisp.png");  // Texture for willowisp
	SDL_Texture* texFinaldoor = IMG_LoadTexture(renderer, "assets/finaldoor.png");
	SDL_Texture* texmetagross = IMG_LoadTexture(renderer, "assets/metagross.png");
	SDL_Texture* texHealth = IMG_LoadTexture(renderer, "assets/health.png");
	SDL_Texture* texReloader = IMG_LoadTexture(renderer, "assets/reloader.png");
	SDL_Texture* megaTexture = IMG_LoadTexture(renderer, "assets/mega_gengar.png");
	SDL_Texture* mixball = IMG_LoadTexture(renderer, "assets/mixballfinal.png");
	TTF_Font* font = TTF_OpenFont("fonthealth.ttf", 35);


	//Final boss definitions
	Boss boss;

	boss.active = true;
	boss.x = 17 * TileSize;   // Top-left of boss, inside the room (can adjust)
	boss.y = 1 * TileSize;
	boss.vx = -0.25f;
	boss.vy = 0.25f;
	boss.rect = { boss.x, boss.y, TileSize * 3, TileSize * 3 };


	//Mimikyu Darkrai direction
	bool mimiDirection=true;
	bool mimiDirection2 = true;
	bool mimiDirection3 = true;
	bool spiritDirection1 = true;
	bool spiritDirection2 = true;

	//For dynamic screen
	camera = {0, 0, window_width, window_height};

	//Defining Characters
	Characters gengar(
	    1 * TileSize + (TileSize - 64) / 2.0f,   // x
	    1 * TileSize + (TileSize - 64) / 2.0f,   // y
	    0, 0,
	{1 * TileSize + (TileSize - 64) / 2.0f, 1 * TileSize + (TileSize - 64) / 2.0f, 64, 64},
	10, true
	);

	Characters mimi(6 * TileSize, 1 * TileSize, 0, 0, {6 * TileSize, 1 * TileSize, TileSize, TileSize}, 1000, true);
	Characters mimi2(1 * TileSize, 4 * TileSize, 0, 0, {1 * TileSize, 4 * TileSize, TileSize, TileSize}, 1000, true);
	Characters mimi3(1 * TileSize, 14 * TileSize, 0, 0, {1 * TileSize, 14 * TileSize, TileSize, TileSize}, 1000, true);
	Characters spirit1(7* TileSize, 19 * TileSize, 0, 0, {7 * TileSize, 19 * TileSize, TileSize, TileSize}, 1000, true);
	Characters spirit2(12 * TileSize,  17 * TileSize, 0, 0, {12 * TileSize, 17 * TileSize, TileSize, TileSize}, 1000, true);
	Characters alakazam(3 * TileSize, 11 * TileSize, 0, 0, {3 * TileSize, 11 * TileSize, 2 * TileSize, 2 * TileSize}, 10, true);
	Characters zekrom(20 * TileSize, 20 * TileSize, 0, 0, {20 * TileSize, 20 * TileSize, 2 * TileSize, 2 * TileSize}, 10, true);
	Characters darkrai(10 * TileSize, 2 * TileSize, 0, 0, {10 * TileSize, 2 * TileSize, TileSize,TileSize}, 5, true);
	Characters darkrai2(8 * TileSize, 15 * TileSize, 0, 0, {8 * TileSize, 15 * TileSize, TileSize, TileSize}, 5, true);
	Characters metagross(12 * TileSize, 14 * TileSize, 0, 0, {12 * TileSize,14  * TileSize, TileSize, TileSize}, 5, true);

	vector<Bullet> bullets;
	vector<Bullet> psyball;
	vector<Bullet> elecBall;
	bool gameIsRunning = true;

	for(int i=0; i<GridHeight; i++)
	{
		for(int j=0; j<GridWidth; j++)
		{
			if(RoomGrid[i][j]==1)
			{
				SDL_FRect rectangle= {j*TileSize, i*TileSize, TileSize, TileSize};
				walls.push_back(rectangle);
			}
		}
	}

	Uint32 lastTicks = SDL_GetTicks();
	vector<bool> justEnteredPortal(portalDefs.size(), false);
	while (gameIsRunning)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				gameIsRunning = false;
			}

			SDL_FRect temp = {gengar.charRect.x, gengar.charRect.y, gengar.charRect.w, gengar.charRect.h};

			const Uint8* state = SDL_GetKeyboardState(NULL);

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_w) temp.y -= 10;
				if (event.key.keysym.sym == SDLK_s) temp.y += 10;
				if (event.key.keysym.sym == SDLK_a) temp.x -= 10;
				if (event.key.keysym.sym == SDLK_d) temp.x += 10;
			}

			if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_D]) {
				temp.y -= 2;
				temp.x += 2;
			}
			if (state[SDL_SCANCODE_S] && state[SDL_SCANCODE_D]) {
				temp.y += 2;
				temp.x += 2;
			}
			if (state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S]) {
				temp.y += 2;
				temp.x -= 2;
			}
			if (state[SDL_SCANCODE_A] && state[SDL_SCANCODE_W]) {
				temp.y -= 2;
				temp.x -= 2;
			}

			if (!collisionWithWall(temp))
			{
				gengar.charRect.x = temp.x;
				gengar.charRect.y = temp.y;
			}
// ---- Portal/Trap Generalized Logic ----
			int centerX = static_cast<int>(gengar.charRect.x + gengar.charRect.w / 2) / TileSize;
			int centerY = static_cast<int>(gengar.charRect.y + gengar.charRect.h / 2) / TileSize;
			for (size_t i = 0; i < portalDefs.size(); ++i) {
				auto& p = portalDefs[i];
				if (!justEnteredPortal[i] &&
				        centerX == p.col && centerY == p.row &&
				        RoomGrid[p.row][p.col] == 2)
				{
					justEnteredPortal[i] = true;
					bool finishedTrap = true;
					if (p.trapFunc) {
						// Save current window size before entering the trap room
						int oldW, oldH;
						SDL_GetWindowSize(window, &oldW, &oldH);
						//Set trap1 room size
						if (p.trapFunc== runTrap1) {
							SDL_SetWindowSize(window, 880, 880);
							SDL_SetWindowTitle(window, "Twinforge Inferno");
						}
						//Set trap2 room size
						if (p.trapFunc == runTrap2) {
							SDL_SetWindowSize(window, 800, 800);
							SDL_SetWindowTitle(window, "Glacial Pitfall");
						}
						// Set trap3 room size
						if (p.trapFunc == runTrap3) {
							SDL_SetWindowSize(window, 960, 800);
							SDL_SetWindowTitle(window, "Voltbound Cryptic Dungeon");
						}
						// Set trap4 room size
						if (p.trapFunc == runTrap4) {

							SDL_SetWindowSize(window, 800, 800);
							SDL_SetWindowTitle(window, "Thorned Threshold");
						}

						finishedTrap = p.trapFunc(window, renderer,gameStartTime, timerRunning);

						// Restore main map window size
						SDL_SetWindowSize(window, oldW, oldH);
						SDL_SetWindowTitle(window, "Gengar Escape: Whispers of Forbidden Memories");
						if (!finishedTrap) {
							gameIsRunning = false;
							break;
						}
					}
					if (finishedTrap) {
						// On returning, portal tile turns to wall, and gengar.charRect moves to next tile
						RoomGrid[p.row][p.col] = 0;
						gengar.charRect.x = p.nextCol * TileSize + (TileSize - gengar.charRect.w) / 2.0f;
						gengar.charRect.y = p.nextRow * TileSize + (TileSize - gengar.charRect.h) / 2.0f;
						if (i == 3) { // 4th portal (0-based index)
							texture = megaTexture;
							shadow=mixball;
						}
					}
				}
				if (justEnteredPortal[i] && !(centerX == p.col && centerY == p.row)) {
					justEnteredPortal[i] = false;

				}
			}

			int gengarCenterX = static_cast<int>(gengar.charRect.x + gengar.charRect.w / 2) / TileSize;
			int gengarCenterY = static_cast<int>(gengar.charRect.y + gengar.charRect.h / 2) / TileSize;
			if (RoomGrid[gengarCenterY][gengarCenterX] == 3) {
				if (boss.health <= 0) {
					if (!isWin) { // Only do this once!
						gameEndTime = SDL_GetTicks();
						timerRunning = false;
					}
					isWin = true;
				} else {
					// Optional: feedback (blink, sound, etc)
				}
			}

//reloader ammunation
			if (RoomGrid[gengarCenterY][gengarCenterX] == 4 && !hasReloader) {
				hasReloader = true;
				RoomGrid[gengarCenterY][gengarCenterX] = 0; // Change reloader tile to regular tile
			}

			//cout << "Gengar coor: " << gengar.charRect.x << " " << gengar.charRect.y << endl;

			camera.x = gengar.charRect.x - window_width / 2; //genger er shathe camera norbe
			camera.y = gengar.charRect.y - window_height / 2;

			if(camera.x<0) camera.x=0;
			if(camera.y<0) camera.y=0;     //Camera k clamp korbe
			if(camera.x>map_width-camera.w) camera.x= map_width- camera.w;
			if(camera.y>map_height-camera.h) camera.y= map_height- camera.h;

			//cout << "On screen Gengar coor: " << gengar.charRect.x - camera.x << " " << gengar.charRect.y - camera.y << endl;

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && hasReloader)
			{
				// Shoot bullet toward mouse
				int mx, my;
				SDL_GetMouseState(&mx, &my);

				mx += camera.x;
				my += camera.y;

				int centerX = (gengar.charRect.x) + gengar.charRect.w / 2;
				int centerY = (gengar.charRect.y) + gengar.charRect.h / 2;

				double angle = atan2(my - centerY, mx - centerX);
				float speed = 0.5f;

				Bullet b;
				b.x = centerX;
				b.y = centerY;
				b.vx = cos(angle) * speed;
				b.vy = sin(angle) * speed;
				b.rect.w = 50;
				b.rect.h = 50;
				b.rect.x = b.x;
				b.rect.y = b.y;

				bullets.push_back(b);
			}
		}
		//Boss movement
		if (boss.active) {

			boss.x += boss.vx;
			boss.y += boss.vy;
			boss.rect.x = boss.x;
			boss.rect.y = boss.y;

			// Bouncing boundaries for final room:
			// For top/bottom: rows 1 to 8 (for top-left of boss, so bottom edge fits inside room)
			// For left/right: cols 12 to 19 (for top-left of boss, so right edge fits inside room)

			float minX = 12 * TileSize;
			float maxX = 19 * TileSize; // 19+3=22, fits inside 21
			float minY = 1 * TileSize;
			float maxY = 8 * TileSize;  // 8+3=11, fits inside 10

			if (boss.x <= minX) {
				boss.x = minX;
				boss.vx = -boss.vx;
			}
			if (boss.x >= maxX) {
				boss.x = maxX;
				boss.vx = -boss.vx;
			}
			if (boss.y <= minY) {
				boss.y = minY;
				boss.vy = -boss.vy;
			}
			if (boss.y >= maxY) {
				boss.y = maxY;
				boss.vy = -boss.vy;
			}

			boss.rect.x = boss.x;
			boss.rect.y = boss.y;
		}

//boss fireball movement
// Time since last emission (3 seconds interval)
		Uint32 currentTime = SDL_GetTicks();

		if (boss.health > 0 && currentTime - lastFireballTime >= fireballEmissionInterval) {
			// Emit 6 willowisp from the boss
			for (int i = 0; i < 6; ++i) {
				// Calculate angle for equally spaced willowisp
				float angle = i * M_PI / 3;  // 6 willowisp in a circle, 60 degrees apart

				// Fireball spawn position around the boss
				float fireballX = boss.x + 1.5f * TileSize * cos(angle);  // Offset boss position
				float fireballY = boss.y + 1.5f * TileSize * sin(angle);

				// Create a new fireball
				BossFireball fireball;
				fireball.x = fireballX;
				fireball.y = fireballY;
				fireball.vx = cos(angle) * 0.5f;  // Speed of fireball
				fireball.vy = sin(angle) * 0.5f;
				fireball.rect = {fireball.x, fireball.y, TileSize, TileSize};
				fireball.active = true;

				willowisp.push_back(fireball);  // Add fireball to the list
			}

			// Update the last emission time
			lastFireballTime = currentTime;
		}






		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);


//rendering starts
		if (isGameOver) {
			// Set the blend mode to allow transparency
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

			// Render black transparent overlay over the entire window (semi-transparent black background)
			SDL_SetRenderDrawColor(renderer, 238, 130, 238, 1);  // Black with 50% opacity (128 out of 255)
			SDL_FRect overlayRect = { 0, 0, window_width, window_height };  // Full screen
			SDL_RenderFillRectF(renderer, &overlayRect);  // Fill entire window with the overlay

			// Load the font and render "Game Over" text
			TTF_Font* fontGameOver = TTF_OpenFont("font.ttf", 100);
			SDL_Color violet = {238, 130, 238};  // Violet color for the text

			// Create the text surface and texture
			string gameOverText = "Game Over";
			SDL_Surface* textSurface = TTF_RenderText_Solid(fontGameOver, gameOverText.c_str(), violet);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

			// Calculate the position to center the text on the screen
			int textWidth = textSurface->w;
			int textHeight = textSurface->h;
			SDL_FRect textRect = {
				static_cast<float>((window_width - textWidth) / 2),  // Center horizontally
				static_cast<float>((window_height - textHeight) / 2), // Center vertically
				static_cast<float>(textWidth),
				static_cast<float>(textHeight)
			};

			// Render the text
			SDL_RenderCopyF(renderer, textTexture, NULL, &textRect);
			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);

			SDL_RenderPresent(renderer);

			SDL_Delay(3000);  // Show Game Over for 3 seconds

			// Clean up font if not needed later


			// End game loop to return to UI menu
			return GameResult{false, 0};

		}

		if (isWin) {
			// Draw light blue transparent overlay
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Light blue, semi-transparent
			SDL_FRect overlayRect = { 0, 0, window_width, window_height };
			SDL_RenderFillRectF(renderer, &overlayRect);

			// --- "YOU WIN!" Text ---
			TTF_Font* fontWin = TTF_OpenFont("fonthealth.ttf", 100);

			//render win text
			SDL_Color winColor = {37, 150, 190}; // Light blue
			string winText = "YOU WIN!";
			SDL_Surface* winSurface = TTF_RenderText_Solid(fontWin, winText.c_str(), winColor);
			SDL_Texture* winTexture = SDL_CreateTextureFromSurface(renderer, winSurface);
			int winTextWidth = winSurface->w;

			int winTextHeight = winSurface->h;
			SDL_FRect winRect = {
				static_cast<float>((window_width - winTextWidth) / 2),
				static_cast<float>((window_height - winTextHeight) / 2) - 50, // slightly higher
				static_cast<float>(winTextWidth),
				static_cast<float>(winTextHeight)
			};
			SDL_RenderCopyF(renderer, winTexture, NULL, &winRect);

			SDL_FreeSurface(winSurface);
			SDL_DestroyTexture(winTexture);

			TTF_CloseFont(fontWin);


			// --- Timer (final time) ---
			Uint32 elapsed = gameEndTime - gameStartTime;
			int minutes = elapsed / 60000;
			int seconds = (elapsed % 60000) / 1000;

			char finalTimeText[32];
			snprintf(finalTimeText, sizeof(finalTimeText), "Time: %02d:%02d", minutes, seconds);

			TTF_Font* fontTime = TTF_OpenFont("fonthealth.ttf", 48);

			SDL_Color color = {255, 255, 255}; // White
			SDL_Surface* timeSurface = TTF_RenderText_Solid(fontTime, finalTimeText, color);

			SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
			SDL_FRect timeRect = {
				static_cast<float>((window_width - timeSurface->w) / 2),
				static_cast<float>((window_height - timeSurface->h) / 2) + 80,
				static_cast<float>(timeSurface->w),
				static_cast<float>(timeSurface->h)
			};
			SDL_RenderCopyF(renderer, timeTexture, NULL, &timeRect);

			SDL_FreeSurface(timeSurface);
			SDL_DestroyTexture(timeTexture);

			TTF_CloseFont(fontTime);


			SDL_RenderPresent(renderer);
			SDL_Delay(3000); // Wait 3 seconds

			Uint32 Elapsed = gameEndTime - gameStartTime;
			int Seconds = elapsed / 1000;
			return GameResult{true, Seconds};
		}






		for (int x = 0; x < GridWidth; x++)
		{
			for (int y = 0; y < GridHeight; y++)
			{
				SDL_FRect tileRect{(x * TileSize- camera.x),(y * TileSize- camera.y), TileSize, TileSize};

				if (RoomGrid[y][x] == 0)
					SDL_RenderCopyF(renderer, texTile, NULL, &tileRect);

				else if (RoomGrid[y][x] == 1)
					SDL_RenderCopyF(renderer, texWall, NULL, &tileRect);

				else if (RoomGrid[y][x] == 2)
					SDL_RenderCopyF(renderer, texPortal, NULL, &tileRect);
				else if (RoomGrid[y][x] == 3)
					SDL_RenderCopyF(renderer, texFinaldoor, NULL, &tileRect);
				else if (RoomGrid[y][x] == 4)  // Add this new condition
					SDL_RenderCopyF(renderer, texReloader, NULL, &tileRect);
			}
		}

		Uint32 elapsed_time= SDL_GetTicks();

		// Get mouse for rotation
		/*int mx, my;
		SDL_GetMouseState(&mx, &my);

		mx += camera.x;
		my += camera.y;

		int centerX = gengar.charRect.x + gengar.charRect.w / 2;
		int centerY = gengar.charRect.y + gengar.charRect.h / 2;
		double angle = atan2(my - centerY, mx - centerX) * (180.0 / M_PI);
		SDL_Point point = {(int) gengar.charRect.w / 2, (int) gengar.charRect.h / 2};*/

		if(gengar.show == true)
		{
			SDL_Rect renderRect{(int)(gengar.charRect.x - camera.x),(int)(gengar.charRect.y- camera.y), (int)gengar.charRect.w, (int)gengar.charRect.h};
			SDL_RenderCopy(renderer, texture, NULL, &renderRect);
		}

		int i = 0;

		while(i < bullets.size())
		{
			bullets[i].x += bullets[i].vx;
			bullets[i].y += bullets[i].vy;
			bullets[i].rect.x = (bullets[i].x);
			bullets[i].rect.y = (bullets[i].y);

			//cout << bullets[i].rect.x << " " << bullets[i].rect.y << endl;
			if (SDL_HasIntersectionF(&bullets[i].rect, &boss.rect) && boss.active == true)
			{
				boss.health--;
				bullets.erase(bullets.begin() + i);
			}

			if (SDL_HasIntersectionF(&bullets[i].rect, &alakazam.charRect) && alakazam.show == true && alakazam.health>0)
			{
				alakazam.health--;
				bullets.erase(bullets.begin() + i);
			}

			else if (SDL_HasIntersectionF(&bullets[i].rect, &darkrai.charRect) && darkrai.show == true)
			{
				darkrai.health--;
				bullets.erase(bullets.begin() + i);
			}

			else if (SDL_HasIntersectionF(&bullets[i].rect, &darkrai2.charRect) && darkrai2.show == true)
			{
				darkrai2.health--;
				bullets.erase(bullets.begin() + i);
			}
			else if (SDL_HasIntersectionF(&bullets[i].rect, &metagross.charRect) && metagross.show == true)
			{
				metagross.health--;
				bullets.erase(bullets.begin() + i);
			}

			else if (SDL_HasIntersectionF(&bullets[i].rect, &zekrom.charRect) && zekrom.show == true)
			{
				zekrom.health--;
				bullets.erase(bullets.begin() + i);
			}

			else if (collisionWithWall(bullets[i].rect))
			{
				bullets.erase(bullets.begin() + i);
			}

			else if (!collisionWithWall(bullets[i].rect))
			{
				SDL_FRect temp{bullets[i].x - camera.x, bullets[i].y - camera.y, 50, 50};
				SDL_RenderCopyF(renderer, shadow, NULL, &temp);
				i++;
			}
		}

		if (alakazam.health <= 0) alakazam.show = false;
		if (zekrom.health <= 0) zekrom.show = false;
		if (darkrai.health <= 0) darkrai.show = false;
		if (darkrai2.health <= 0) darkrai2.show = false;
		if(metagross.health<=0) metagross.show =false;
		if(boss.health<=0) {
			boss.active=false;

			for (int i = 0; i < willowisp.size(); ++i) {
				willowisp[i].active = false;  // Deactivate all fireballs
			}
			willowisp.clear();
		}
		if(mimi.show == true)
		{
			if(mimiDirection == true) mimi.charRect.y += TileSize / 800;
			if(mimiDirection == false) mimi.charRect.y -= TileSize / 800;
			if(mimi.charRect.y <= 1 * TileSize) mimiDirection = true;
			if(mimi.charRect.y >= 6 * TileSize) mimiDirection = false;

			SDL_FRect relMimi{mimi.charRect.x - camera.x, mimi.charRect.y - camera.y, 80, 80};
			SDL_RenderCopyF(renderer, texMimi, NULL, &relMimi);
		}

		if (mimi2.show == true)
		{
			if(mimiDirection2 == true) mimi2.charRect.x += TileSize / 800;
			if(mimiDirection2 == false) mimi2.charRect.x -= TileSize / 800;
			if(mimi2.charRect.x <= 1 * TileSize) mimiDirection2 = true;
			if(mimi2.charRect.x >= 6 * TileSize) mimiDirection2 = false;

			SDL_FRect relMimi{mimi2.charRect.x - camera.x, mimi2.charRect.y - camera.y, 80, 80};
			SDL_RenderCopyF(renderer, texMimi, NULL, &relMimi);
		}

		if (mimi3.show == true)
		{
			if(mimiDirection3 == true) mimi3.charRect.x += TileSize / 500;
			if(mimiDirection3 == false) mimi3.charRect.x -= TileSize / 500;
			if(mimi3.charRect.x <= 1 * TileSize) mimiDirection3 = true;
			if(mimi3.charRect.x >= 6 * TileSize) mimiDirection3 = false;

			SDL_FRect relMimi{mimi3.charRect.x - camera.x, mimi3.charRect.y - camera.y, 80, 80};
			SDL_RenderCopyF(renderer, texMimi, NULL, &relMimi);
		}

		if (spirit1.show == true)
		{
			if(spiritDirection1 == true) spirit1.charRect.x += TileSize / 400;
			if(spiritDirection1 == false) spirit1.charRect.x -= TileSize / 400;
			if(spirit1.charRect.x <= 7 * TileSize) spiritDirection1 = true;
			if(spirit1.charRect.x >= 19 * TileSize) spiritDirection1 = false;

			SDL_FRect relspirit{spirit1.charRect.x - camera.x, spirit1.charRect.y - camera.y, 80, 80};
			SDL_RenderCopyF(renderer, texSpirit, NULL, &relspirit);
		}

		if (spirit2.show == true)
		{
			if(spiritDirection2 == true) spirit2.charRect.y += TileSize / 400;
			if(spiritDirection2 == false) spirit2.charRect.y -= TileSize / 400;
			if(spirit2.charRect.y <= 17 * TileSize) spiritDirection2 = true;
			if(spirit2.charRect.y >= 21 * TileSize) spiritDirection2 = false;

			SDL_FRect relspirit{spirit2.charRect.x - camera.x, spirit2.charRect.y - camera.y, 80, 80};
			SDL_RenderCopyF(renderer, texSpirit, NULL, &relspirit);
		}

		if((SDL_HasIntersectionF(&mimi.charRect, &gengar.charRect)) || (SDL_HasIntersectionF(&mimi2.charRect, &gengar.charRect)))
			respawn(gengar, 1, 1);

		if((SDL_HasIntersectionF(&mimi3.charRect, &gengar.charRect)))
			respawn(gengar, 8, 3);

		if((SDL_HasIntersectionF(&spirit1.charRect, &gengar.charRect)) || (SDL_HasIntersectionF(&spirit2.charRect, &gengar.charRect)))
			respawn(gengar, 2, 19);
		if((SDL_HasIntersectionF(&alakazam.charRect, &gengar.charRect)) && alakazam.health>0)
			respawn(gengar, 11, 6);
		if((SDL_HasIntersectionF(&zekrom.charRect, &gengar.charRect)) && zekrom.health>0)
			respawn(gengar, 4, 19);


		if(alakazam.show == true && alakazam.health>0)
		{
			SDL_FRect temp{alakazam.charRect.x - camera.x, alakazam.charRect.y - camera.y, TileSize * 2, TileSize * 2};
			SDL_RenderCopyF(renderer, texAlakazam, NULL, &temp);
		}

		if (alakazam.show == true && elapsed_time % 2000 == 0)
		{
			int centerX = alakazam.charRect.x + alakazam.charRect.w / 2;
			int centerY = alakazam.charRect.y + alakazam.charRect.h / 2;

			double angle = atan2(gengar.charRect.y - centerY, gengar.charRect.x - centerX);
			float speed = 0.65f;

			Bullet b;
			b.x = centerX;
			b.y = centerY;
			b.vx = cos(angle) * speed;
			b.vy = sin(angle) * speed;
			b.rect.w = 50;
			b.rect.h = 50;
			b.rect.x = b.x;
			b.rect.y = b.y;

			psyball.push_back(b);
		}

		i = 0;

		while(i < psyball.size())
		{
			psyball[i].x += psyball[i].vx;
			psyball[i].y += psyball[i].vy;
			psyball[i].rect.x = (psyball[i].x);
			psyball[i].rect.y = (psyball[i].y);

			if (SDL_HasIntersectionF(&gengar.charRect, &psyball[i].rect))
			{
				totalHits++;  // Increment the counter for elecball hits
				if (totalHits>=5) {  // If 5 hits, decrease health by 1
					gengar.health--;
					totalHits=0;
					// Reset the hit counter
				}
				if (gengar.health <= 0) {
					gengar.show = false;     // Hide Gengar
					gengar.charRect.x = -100; // Move off-screen
					gengar.charRect.y = -100;
					isGameOver = true;
				}
				psyball.erase(psyball.begin() + i);
			}



			else if (collisionWithWall(psyball[i].rect))
				psyball.erase(psyball.begin() + i);

			else if (!collisionWithWall(psyball[i].rect))
			{
				SDL_FRect temp{psyball[i].x - camera.x, psyball[i].y - camera.y, 50, 50};
				SDL_RenderCopyF(renderer, texPsyball, NULL, &temp);
				i++;
			}
		}

		if(zekrom.show == true && zekrom.health>0)
		{
			SDL_FRect temp{zekrom.charRect.x - camera.x, zekrom.charRect.y - camera.y, TileSize * 2, TileSize * 2};
			SDL_RenderCopyF(renderer, texZekrom, NULL, &temp);
		}

		if (zekrom.show == true && elapsed_time % 2000 == 0)
		{
			int centerX = zekrom.charRect.x + zekrom.charRect.w / 2;
			int centerY = zekrom.charRect.y + zekrom.charRect.h / 2;

			double angle = atan2(gengar.charRect.y - centerY, gengar.charRect.x - centerX);
			float speed = 0.65f;

			Bullet b;
			b.x = centerX;
			b.y = centerY;
			b.vx = cos(angle) * speed;
			b.vy = sin(angle) * speed;
			b.rect.w = 50;
			b.rect.h = 50;
			b.rect.x = b.x;
			b.rect.y = b.y;

			elecBall.push_back(b);
		}

		i = 0;

		while(i < elecBall.size())
		{
			elecBall[i].x += elecBall[i].vx;
			elecBall[i].y += elecBall[i].vy;
			elecBall[i].rect.x = (elecBall[i].x);
			elecBall[i].rect.y = (elecBall[i].y);

			//cout << elecBall[i].rect.x << " " << elecBall[i].rect.y << endl;

			if (SDL_HasIntersectionF(&gengar.charRect, &elecBall[i].rect))
			{
				totalHits++;  // Increment the counter for elecball hits
				if (totalHits >= 5) {  // If 5 hits, decrease health by 1
					gengar.health--;
					totalHits = 0;  // Reset the hit counter
				}
				if (gengar.health <= 0) {
					gengar.show = false;     // Hide Gengar
					gengar.charRect.x = -100; // Move off-screen
					gengar.charRect.y = -100;
					isGameOver = true;
				}
				elecBall.erase(elecBall.begin() + i);
			}



			else if (collisionWithWall(elecBall[i].rect))
				elecBall.erase(elecBall.begin() + i);

			else if (!collisionWithWall(elecBall[i].rect))
			{
				SDL_FRect temp{elecBall[i].x - camera.x, elecBall[i].y - camera.y, 50, 50};
				SDL_RenderCopyF(renderer, texElecBall, NULL, &temp);
				i++;
			}
		}

		if (darkrai.show == true)
		{
			int centerX = darkrai.charRect.x + darkrai.charRect.w / 2;
			int centerY = darkrai.charRect.y + darkrai.charRect.h / 2;

			double angle = atan2(gengar.charRect.y - centerY, gengar.charRect.x - centerX);
			float speed = 0.05f;

			float vx = cos(angle) * speed;
			float vy = sin(angle) * speed;

			SDL_FRect intersection{darkrai.charRect.x, darkrai.charRect.y, TileSize, TileSize};

			intersection.x += vx;
			intersection.y += vy;

			if (!collisionWithWall(intersection))
			{
				darkrai.charRect.x += vx;
				darkrai.charRect.y += vy;
			}

			if (SDL_HasIntersectionF(&gengar.charRect, &darkrai.charRect))
				respawn(gengar, 1, 8);

			SDL_FRect temp{darkrai.charRect.x - camera.x, darkrai.charRect.y - camera.y, TileSize, TileSize};
			SDL_RenderCopyF(renderer, texDarkrai, NULL, &temp);
		}


		if (darkrai2.show == true)
		{
			int centerX = darkrai2.charRect.x + darkrai2.charRect.w / 2;
			int centerY = darkrai2.charRect.y + darkrai2.charRect.h / 2;

			double angle = atan2(gengar.charRect.y - centerY, gengar.charRect.x - centerX);
			float speed = 0.05f;

			float vx = cos(angle) * speed;
			float vy = sin(angle) * speed;

			SDL_FRect intersection{darkrai2.charRect.x, darkrai2.charRect.y, TileSize, TileSize};

			intersection.x += vx;
			intersection.y += vy;

			if (!collisionWithWall(intersection))
			{
				darkrai2.charRect.x += vx;
				darkrai2.charRect.y += vy;
			}

			if (SDL_HasIntersectionF(&gengar.charRect, &darkrai2.charRect))
				respawn(gengar, 1, 8);

			SDL_FRect temp{darkrai2.charRect.x - camera.x, darkrai2.charRect.y - camera.y, TileSize, TileSize};
			SDL_RenderCopyF(renderer, texDarkrai, NULL, &temp);
		}
		if (metagross.show == true)
		{
			int centerX = metagross.charRect.x + metagross.charRect.w / 2;
			int centerY = metagross.charRect.y + metagross.charRect.h / 2;

			double angle = atan2(gengar.charRect.y - centerY, gengar.charRect.x - centerX);
			float speed = 0.05f;

			float vx = cos(angle) * speed;
			float vy = sin(angle) * speed;

			SDL_FRect intersection{metagross.charRect.x, metagross.charRect.y, TileSize, TileSize};

			intersection.x += vx;
			intersection.y += vy;

			if (!collisionWithWall(intersection))
			{
				metagross.charRect.x += vx;
				metagross.charRect.y += vy;
			}

			if (SDL_HasIntersectionF(&gengar.charRect, &metagross.charRect))
				respawn(gengar, 21, 15);

			SDL_FRect temp{metagross.charRect.x - camera.x, metagross.charRect.y - camera.y, TileSize, TileSize};
			SDL_RenderCopyF(renderer, texmetagross, NULL, &temp);
		}


		for (int i = 0; i < willowisp.size(); ++i) {
			if (willowisp[i].active) {
				// Move the fireball
				willowisp[i].x += willowisp[i].vx;
				willowisp[i].y += willowisp[i].vy;

				// Update the fireball's rectangle
				willowisp[i].rect.x = willowisp[i].x;
				willowisp[i].rect.y = willowisp[i].y;

				// Check for collision with walls
				if (collisionWithWall(willowisp[i].rect)) {
					willowisp[i].active = false;  // Deactivate the fireball if it hits the wall

				}

				if (willowisp[i].active && SDL_HasIntersectionF(&willowisp[i].rect, &gengar.charRect)) {
					gengar.health--;
					if (gengar.health <= 0) {
						gengar.show = false;     // Hide Gengar
						gengar.charRect.x = -100; // Move off-screen
						gengar.charRect.y = -100;
						isGameOver = true;
					}  // Decrease Gengar's health by 1
					willowisp[i].active = false;  // Deactivate the willowisp if it hits Gengar
				}


				// Render the fireball if it's active
				if (willowisp[i].active) {
					SDL_FRect fireballRect{willowisp[i].rect.x - camera.x, willowisp[i].rect.y - camera.y, willowisp[i].rect.w, willowisp[i].rect.h};
					SDL_RenderCopyF(renderer, texWillowisp, NULL, &fireballRect);  // Render the fireball
				}
			}
		}

		if (boss.active && SDL_HasIntersectionF(&gengar.charRect, &boss.rect) && boss.health>0) {
			respawn(gengar,21,10);   // Gengar respawns at (10,12)
		}


		if (boss.active && boss.health>0) {
			SDL_FRect bossDrawRect = { boss.x - camera.x, boss.y - camera.y, boss.rect.w, boss.rect.h };
			SDL_RenderCopyF(renderer, bossTexture, NULL, &bossDrawRect);
		}
		if (gengar.show) {
			// Calculate the position for the heart image (top-right corner)
			SDL_FRect heartRect = { window_width - 120, 20, 50, 50 };  // Adjust this as needed

			// Render the heart image (health indicator)
			SDL_RenderCopyF(renderer, texHealth, nullptr, &heartRect);

			// Prepare the text to show the health number
			SDL_Color textColor = {255, 255, 255};  // White color for the health number
			string healthText = std::to_string(gengar.health);  // Convert health to string

			// Render the health number next to the heart image
			SDL_Surface* textSurface = TTF_RenderText_Solid(font, healthText.c_str(), textColor);

			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			// Position for the text (next to the heart image)
			SDL_Rect textRect = { window_width - 52, 26, textSurface->w, textSurface->h }; // Adjust position as needed
			SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);

		}

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


	SDL_Delay(3000);

	Mix_CloseAudio();


	return GameResult{false, 0};


}

