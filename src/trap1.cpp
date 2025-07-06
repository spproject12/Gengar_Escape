#include <iostream>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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



int tilesize = 80;
int gridwidth = 11;
int gridheight = 11;
const int window_Width = gridwidth*tilesize;
const int window_Height = gridheight*tilesize;
// 0= emptyTile, 1= inactiveSwitch, 2= MagmaTile, 3=beginningTile, 4=ActiveSwitch, 6=FireStone
int roomgrid[11][11] = {
	{0,0,0,0,0,1,0,0,0,0,3},
	{0,0,0,0,0,6,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,6,0,0,6,0,0,6,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{1,0,6,0,0,0,0,0,6,2,1},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,6,0,0,6,0,0,6,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,6,0,0,0,0,0},
	{3,0,0,0,0,1,0,0,0,0,0},
};

int roomFirestone[11][11] = {
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,6,0,0,0,0},
	{0,0,6,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,6,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,6,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0}
};

int getRandomOneToFour()
{
	return (rand() % 4) + 1;
}

int getRandomOneToTen()
{
	return (rand() % 10) + 1;
}

// ---- TRAP ROOM FUNCTION ----
bool runTrap1(SDL_Window* window, SDL_Renderer* renderer,Uint32 gameStartTime, bool timerRunning)
{
	srand(static_cast<unsigned int>(time(0)));

	SDL_Texture* texGengar = IMG_LoadTexture(renderer, "assets/gengar.png");
	SDL_Texture* texShadowBall = IMG_LoadTexture(renderer, "assets/bullet.png");
	SDL_Texture* texTile = IMG_LoadTexture(renderer, "assets/normaltile.png");
	SDL_Texture* texDarkGengar = IMG_LoadTexture(renderer, "assets/Dark_gengar.png");
	SDL_Texture* texSwitch = IMG_LoadTexture(renderer, "assets/stoneSwitch.png");
	SDL_Texture* texLavaTile = IMG_LoadTexture(renderer, "assets/Lava_tile.png");
	SDL_Texture* texIce = IMG_LoadTexture(renderer, "assets/normaltile.png");
	SDL_Texture* doorTex = IMG_LoadTexture(renderer, "assets/door.png");
	SDL_Texture* FireStone = IMG_LoadTexture(renderer, "assets/firestone.png");

	Characters gengar(10 * tilesize, 0 * tilesize, 0, 0, {10 * tilesize, 0 * tilesize, tilesize, tilesize}, 5, true);
	Characters darkGengar(0 * tilesize, 10 * tilesize, 0, 0, {0 * tilesize, 10 * tilesize, tilesize, tilesize}, 5, true);
	bool MagmaShow = false;

	bool gameIsRunning = true;
	vector<Bullet> bullets;
	Uint64 startTime = SDL_GetTicks64();
	int elaspedTime;
	int duration = 3000;
	int buttonNumber = getRandomOneToFour(); //Random button number dibe
	SDL_Rect DoorRect = {0,0,tilesize,tilesize};
	bool doorShow = false;
	int doorStart = SDL_GetTicks64();
	int doorRow, doorCol;
	bool stoneShow = true;
	int stoneNum = 10;


	// ---- TRAP ROOM MAIN LOOP ----
	while (gameIsRunning)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				return false; // quit entirely
			}

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP)
				{
					if (gengar.charRect.y - tilesize >= 0)
					{
						gengar.charRect.y -= tilesize;
						darkGengar.charRect.y += tilesize;
					}
				}

				if (event.key.keysym.sym == SDLK_s ||event.key.keysym.sym == SDLK_DOWN)
				{
					if (gengar.charRect.y + tilesize < 880)
					{
						gengar.charRect.y += tilesize;
						darkGengar.charRect.y -= tilesize;
					}
				}

				if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT)
				{
					if (gengar.charRect.x - tilesize >= 0)
					{
						gengar.charRect.x -= tilesize;
						darkGengar.charRect.x += tilesize;
					}
				}

				if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT)
				{
					if (gengar.charRect.x + tilesize < 880)
					{
						gengar.charRect.x += tilesize;
						darkGengar.charRect.x -= tilesize;
					}
				}
			}

			// if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
			// {
			// 	// Shoot bullet toward mouse
			// 	int mx, my;
			// 	SDL_GetMouseState(&mx, &my);

			// 	int centerX = gengar.charRect.x + gengar.charRect.w / 2;
			// 	int centerY = gengar.charRect.y + gengar.charRect.h / 2;

			// 	double angle = atan2(my - centerY, mx - centerX);
			// 	float speed = 1.5f;

			// 	Bullet b;
			// 	b.x = centerX;
			// 	b.y = centerY;
			// 	b.vx = cos(angle) * speed;
			// 	b.vy = sin(angle) * speed;
			// 	b.rect.w = 50;
			// 	b.rect.h = 50;
			// 	b.rect.x = b.x;
			// 	b.rect.y = b.y;

			// 	bullets.push_back(b);
			// }
		}
		// Check collision between Gengar and Dark Gengar
		if (gengar.show && darkGengar.show &&
		        SDL_HasIntersection(&gengar.charRect, &darkGengar.charRect))
		{
			gengar.charRect.x = 10 * tilesize;
			gengar.charRect.y = 0 * tilesize;
			darkGengar.charRect.x = 0 * tilesize;
			darkGengar.charRect.y = 10 * tilesize;
			gengar.health = 5;
			gengar.show = true;
			darkGengar.show = true;
		}


		SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		for (int x = 0; x < gridwidth; x++)
		{
			for (int y = 0; y < gridheight; y++)
			{
				SDL_Rect tileRect{x * tilesize, y * tilesize, tilesize, tilesize};

				if (roomgrid[y][x] == 0)
					SDL_RenderCopy(renderer, texTile, NULL, &tileRect);

				else if (roomgrid[y][x] == 1 || roomgrid[y][x] == 4)
					SDL_RenderCopy(renderer, texSwitch, NULL, &tileRect);

				else if (roomgrid[y][x] == 2 && MagmaShow == true)
					SDL_RenderCopy(renderer, texLavaTile, NULL, &tileRect);

				else if (roomgrid[y][x] == 2 && MagmaShow == false)
					SDL_RenderCopy(renderer, texTile, NULL, &tileRect);

				else if (roomgrid[y][x] == 3)
					SDL_RenderCopy(renderer, texIce, NULL, &tileRect);

				else if (roomgrid[y][x] == 6)
				{
					SDL_RenderCopy(renderer, texTile, NULL, &tileRect);
					if (stoneShow == true && stoneNum > 0) SDL_RenderCopy(renderer, FireStone, NULL, &tileRect);
				}
			}
		}

		elaspedTime = SDL_GetTicks64();



		if (gengar.show == true)
			SDL_RenderCopy(renderer, texGengar, NULL, &gengar.charRect);


		if (darkGengar.show == true)
			SDL_RenderCopy(renderer, texDarkGengar, NULL, &darkGengar.charRect);


		// for (auto& b : bullets)
		// {
		// 	b.x += b.vx;
		// 	b.y += b.vy;
		// 	b.rect.x = static_cast<int>(b.x);
		// 	b.rect.y = static_cast<int>(b.y);
		// 	SDL_RenderCopy(renderer, texShadowBall, NULL, &b.rect);
		// }

		// for (auto it = bullets.begin(); it != bullets.end(); )
		// {
		// 	if (SDL_HasIntersection(&it->rect, &darkGengar.charRect) && darkGengar.show == true)
		// 	{
		// 		bullets.erase(it);
		// 		gengar.health--;
		// 		if (gengar.health <= 0)
		// 		{
		// 			gengar.show = false;
		// 		}
		// 	}
		// 	else it++;
		// }

		int gx = gengar.charRect.x / tilesize; //col
		int gy = gengar.charRect.y / tilesize; //row
		int dgx = darkGengar.charRect.x / tilesize;
		int dgy = darkGengar.charRect.y / tilesize;

		if (roomgrid[gy][gx] == 2 && MagmaShow == true)
		{
			gengar.charRect.x = 10 * tilesize;
			gengar.charRect.y = 0 * tilesize;
			darkGengar.charRect.x = 0 * tilesize;
			darkGengar.charRect.y = 10 * tilesize;
			gengar.health = 5;
			gengar.show = true;
		}

		if(roomgrid[gy][gx] == 6)
		{
			SDL_Rect tileRect1{gx * tilesize, gy * tilesize, tilesize, tilesize};
			roomgrid[gy][gx] = 0;
			SDL_RenderCopy(renderer, texGengar, NULL, &tileRect1);
		}

		if (gengar.show == false)
		{
			gengar.charRect.x = 10 * tilesize;
			gengar.charRect.y = 0 * tilesize;
			darkGengar.charRect.x = 0 * tilesize;
			darkGengar.charRect.y = 10 * tilesize;
			gengar.health = 5;
			gengar.show = true;
		}

		if(MagmaShow == true && gy == 5 && gx == 1)
		{
			darkGengar.show = false; //Dark gengar lavay porle more jabe
			doorShow = true;
			doorStart = SDL_GetTicks64();
			doorRow = getRandomOneToTen();
			doorCol = getRandomOneToTen();
			MagmaShow = false;
		}

		if (doorShow == true && elaspedTime - doorStart <= 3000)
		{
			SDL_Rect tileRect{doorRow * tilesize, doorCol * tilesize, tilesize, tilesize};
			SDL_RenderCopy(renderer, doorTex, NULL, &tileRect);
		}

		// EXIT TO MAIN MAP when player reaches door
		if (doorShow == true && gx == doorRow && gy == doorCol)
		{
			// --- On touching the door, return to main map! ---
			SDL_DestroyTexture(texGengar);
			SDL_DestroyTexture(texShadowBall);
			SDL_DestroyTexture(texTile);
			SDL_DestroyTexture(texDarkGengar);
			SDL_DestroyTexture(texSwitch);
			SDL_DestroyTexture(texLavaTile);
			SDL_DestroyTexture(texIce);
			SDL_DestroyTexture(doorTex);
			SDL_DestroyTexture(FireStone);
			return true; // success, go back to main map and block the portal tile

		}

		if (doorShow == true && elaspedTime - doorStart > 3000)
		{
			doorShow = false;
			darkGengar.show = true;
		}

		if(buttonNumber == 1) {
			if(gy == 0 && gx == 5) roomgrid[0][5] = 4;
		}
		else if(buttonNumber == 2) {
			if(gy ==5  && gx == 10) roomgrid[5][10] = 4;
		}
		else if(buttonNumber == 3) {
			if(gy == 10 && gx == 5) roomgrid[10][5] = 4;
		}
		else if(buttonNumber == 4) {
			if(gy == 5 && gx == 0) roomgrid[5][0] = 4;
		}
		bool nearSwitch = false; //Switch off
		if (roomgrid[gy][gx] == 4)     //Gengar switch er upor ashle, switch pushed hobe
		{
			nearSwitch = true; //switch on hoilo
		}

		if (nearSwitch && MagmaShow == false) {
			MagmaShow = true;
			startTime = SDL_GetTicks64();

			// Lava tile er position
			roomgrid[5][9] = 2;
		}

		if((elaspedTime - startTime) > duration && MagmaShow == true) {
			if(roomgrid[5][9] == 2) roomgrid[5][9] = 0;
			MagmaShow = false;
			buttonNumber = getRandomOneToFour();
		}

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

	SDL_DestroyTexture(texGengar);
	SDL_DestroyTexture(texShadowBall);
	SDL_DestroyTexture(texTile);
	SDL_DestroyTexture(texDarkGengar);
	SDL_DestroyTexture(texSwitch);
	SDL_DestroyTexture(texLavaTile);
	SDL_DestroyTexture(texIce);
	SDL_DestroyTexture(doorTex);
	SDL_DestroyTexture(FireStone);
	return true;
}


