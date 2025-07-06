#include "trap3.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL_ttf.h>
#include "music.h"
using namespace std;



const int TILE_SIZE     = 80;
const int MAP_ROWS      = 10;
const int MAP_COLS      = 12;
const int SCREEN_WIDTH  = TILE_SIZE * MAP_COLS;
const int SCREEN_HEIGHT = TILE_SIZE * MAP_ROWS;

enum TileType {
	WALL       = 1,
	STATUE     = 2,
	LASER      = 3,
	NEWSTATUE  = 4,
	FLOOR      = 0,
	DOOR       = 5,
	OPENDOOR   = 6
};

int tilemap[MAP_ROWS][MAP_COLS] = {
	{1,1,1,1,1,3,3,1,1,1,1,1},
	{1,1,1,1,1,3,3,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,2,0,2,0,0,2,0,2,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,2,0,2,0,0,2,0,2,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,2,0,2,0,0,2,0,2,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1}
};

struct Fireball {
	int row;
	float x;
	int dir;
	bool active;
};
vector<Fireball> fireballs;
bool fireballsActive = false;
const float FIREBALL_SPEED  = 8.0f;
Uint32 fireballTriggerTime  = 0;
const Uint32 FIREBALL_DELAY = 0;

const int NEW_LEFT_COL  = 5;
const int NEW_RIGHT_COL = 6;

bool playerDead   = false;
int playerPrevX, playerPrevY;

struct Puzzle {
	SDL_Texture* image;
	string answer;
	bool solved;
};

const int NUM_SETS        = 4;
const int PUZZLES_PER_SET = 6;

vector< vector<Puzzle>> puzzleSets;
int chosenSetIndex = 0;
vector<Puzzle> puzzles;

bool   puzzle5Solved     = false;
bool   puzzle5Mode       = false;
bool   puzzle5Minimized  = false;
string puzzle5Text;
int    puzzle5Cursor     = 0;
int    puzzle5Scroll     = 0;
bool   puzzle5Wrong      = false;

bool   puzzle6Solved     = false;
bool   puzzle6Mode       = false;
bool   puzzle6Minimized  = false;
string puzzle6Text;
int    puzzle6Cursor     = 0;
int    puzzle6Scroll     = 0;
bool   puzzle6Wrong      = false;
int    puzzle6Col        = -1;

bool   puzzleMode       = false;
bool   puzzleMinimized  = false;
int    currentPuzzleIdx = -1;
string puzzleText;
int    puzzleCursor     = 0;
int    puzzleScroll     = 0;
bool   showWrong        = false;

bool insertedNewStatues = false;

vector< pair<int,int>> stonePositions = {
	{2, 2}, {2, 9}, {3, 5}, {4, 3}, {4, 8},
	{6, 6}, {6, 10}, {7, 1}, {8, 4}, {8, 7}
};
vector<bool> stoneCollected;
SDL_Texture* texStone = nullptr;
int totalStones = 0;
int collectedCount = 0;

TTF_Font* font = nullptr;

SDL_Texture* texLeftDoor  = nullptr;
SDL_Texture* texRightDoor = nullptr;

// ========== UTILITY FUNCTIONS (UNCHANGED) ==========

SDL_Texture* loadTexture(SDL_Renderer* ren, const char* path) {
	SDL_Surface* surf = IMG_Load(path);

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	SDL_FreeSurface(surf);
	return tex;
}

SDL_Texture* renderText(SDL_Renderer* ren, const  string& text, SDL_Color c) {
	SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), c);

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	SDL_FreeSurface(surf);
	return tex;
}

void findPlayerStart(int &px, int &py) {
	for (int y = 0; y < MAP_ROWS; y++) {
		for (int x = 0; x < MAP_COLS; x++) {
			if (tilemap[y][x] == FLOOR) {
				px = x;
				py = y;
				return;
			}
		}
	}
}

void spawnFireballs() {
	fireballs.clear();
	fireballs.push_back({2, 0.0f, +1, true});
	fireballs.push_back({4, (float)(MAP_COLS - 1), -1, true});
	fireballs.push_back({6, 0.0f, +1, true});
	fireballs.push_back({8, (float)(MAP_COLS - 1), -1, true});
	fireballsActive = true;
}

bool fireballsDone() {
	for (auto& f : fireballs) if (f.active) return false;
	return true;
}

void respawn(int &px, int &py) {
	px = playerPrevX;
	py = playerPrevY;
	playerDead = false;
}

int correctStatue(int row, int col) {
	static  vector< pair<int,int>> rightSwitches = {
		{5,4}, {5,7}, {7,2}, {7,9}
	};
	for (size_t i = 0; i < rightSwitches.size(); i++) {
		if (rightSwitches[i].first == row && rightSwitches[i].second == col)
			return (int)i;
	}
	return -1;
}


// ========== MAIN TRAP ROOM FUNCTION ==========

bool runTrap3(SDL_Window* win, SDL_Renderer* ren,Uint32 gameStartTime, bool timerRunning) {

	SDL_Texture* texWall      = loadTexture(ren, "assets/wall.png");
	SDL_Texture* texLaser     = loadTexture(ren, "assets/laser.png");
	SDL_Texture* texStatue    = loadTexture(ren, "assets/statue.png");
	SDL_Texture* texTile      = loadTexture(ren, "assets/tile.png");
	SDL_Texture* texGengar    = loadTexture(ren, "assets/gengar.png");
	SDL_Texture* texFireball  = loadTexture(ren, "assets/fireball.png");
	SDL_Texture* texNewStatue = loadTexture(ren, "assets/newstatue.png");
	SDL_Texture* texDoorLeft  = loadTexture(ren, "assets/leftdoor.png");
	SDL_Texture* texDoorRight = loadTexture(ren, "assets/rightdoor.png");
	SDL_Texture* texOpenDoor  = loadTexture(ren, "assets/opendoor.png");
	SDL_Texture* puzzleSetsImgArr[NUM_SETS][PUZZLES_PER_SET];
                 texStone = loadTexture(ren, "assets/stone.png");
	             font     = TTF_OpenFont("font.ttf", 32);
	
	// We will load 24 puzzle images:(using array to avoid 24 lines of code)

	string setPrefixes[NUM_SETS] = { "assets/puzzleA", "assets/puzzleB", "assets/puzzleC", "assets/puzzleD" };
	for (int s = 0; s < NUM_SETS; s++) {
		for (int p = 0; p < PUZZLES_PER_SET; p++) {
			// Build filename, e.g. "assets/puzzleA1.png"
			string filename = setPrefixes[s] +  to_string(p + 1) + ".png";
			puzzleSetsImgArr[s][p] = loadTexture(ren, filename.c_str());

		}
	}

	

	// ------------------------ Initialize Puzzle Answer Sets ------------------
	//   For each set (A, B, C, D), we define an array of 6 answer strings.
	//   indices 0-3: statue puzzles (puzzles 1-4)
	//   index 4: puzzle 5 answer
	//   index 5: puzzle 6 answer
	//   These answers will be compared case-insensitive

	string answersA[PUZZLES_PER_SET] = {
		"renegade",       // puzzleA1
		"captivity",      // puzzleA2
		"sludge bomb",    // puzzleA3
		"max terror",     // puzzleA4
		"bomb captivity max renegade sludge terror",  // puzzleA5
		"resentment?avarice?jealousy?dejection"       // puzzleA6
	};
	string answersB[PUZZLES_PER_SET] = {
		"levitation",       // puzzleB1
		"obstacle",         // puzzleB2
		"dark pulse",       // puzzleB3
		"max phantasm",     // puzzleB4
		"dark levitation max obstacle phantasm pulse",// puzzleB5
		"indignation?rapacity?spite?despair"         // puzzleB6
	};
	string answersC[PUZZLES_PER_SET] = {
		"pressure",         // puzzleC1
		"trapped",          // puzzleC2
		"night shade",      // puzzleC3
		"max darkness",     // puzzleC4 (just one phrase; combination shown next)
		"darkness max night pressure shade trapped", // puzzleC5
		"dudgeon?avaritia?envy?woe"                  // puzzleC6
	};
	string answersD[PUZZLES_PER_SET] = {
		"shadow force",     // puzzleD1
		"isolated",         // puzzleD2
		"shadow claw",      // puzzleD3
		"max mindstorm",    // puzzleD4 (plus combination string below)
		"claw force isolated max mindstorm shadow shadow", // puzzleD5
		"rancour?greed?animosity?desolation"             // puzzleD6
	};

	// Build puzzleSets vector by pairing each loaded texture with its answer
	puzzleSets.resize(NUM_SETS);
	for (int s = 0; s < NUM_SETS; s++) {
		puzzleSets[s].resize(PUZZLES_PER_SET);
		for (int p = 0; p < PUZZLES_PER_SET; p++) {
			puzzleSets[s][p].image  = puzzleSetsImgArr[s][p];  // Texture
			puzzleSets[s][p].solved = false;                   // Initially unsolved
			// Assign the answer string from the correct array
			switch (s) {
			case 0:
				puzzleSets[s][p].answer = answersA[p];
				break;
			case 1:
				puzzleSets[s][p].answer = answersB[p];
				break;
			case 2:
				puzzleSets[s][p].answer = answersC[p];
				break;
			case 3:
				puzzleSets[s][p].answer = answersD[p];
				break;
			}
		}
	}

	//randomize puzzle set
	srand((unsigned)time(nullptr));
	chosenSetIndex = rand() % NUM_SETS;  // 0=A,1=B,2=C,3=D

	// ------------------- Prepare Puzzles 1-4 for Gameplay --------------------

	puzzles.clear();
	for (int i = 0; i < 4; i++) {
		puzzles.push_back(puzzleSets[chosenSetIndex][i]);
	}
	// Puzzle 5 and Puzzle 6 images/answers remain at puzzleSets[chosenSetIndex][4] and [5]

	// -------------------------- Initialize Stones -----------------------------
	totalStones = (int)stonePositions.size();        // Should be 10
	stoneCollected.resize(totalStones, false);       // None collected at start

	// ---------------------- Find Player Start Position ------------------------
	int playerX, playerY;
	findPlayerStart(playerX, playerY);
	playerPrevX = playerX;
	playerPrevY = playerY;

	// --------------------------- Main Game Loop -------------------------------
	Uint32 lastTick = SDL_GetTicks();            // For computing delta time
	bool running    = true;
	SDL_Event e;
	bool fireballQueued = false;  // True if a fireball spawn is queued
	bool readyToExit = false;
	// Initialize overlay/puzzle state flags
	puzzleMode       = false;
	puzzleMinimized  = false;
	currentPuzzleIdx = -1;
	puzzleText.clear();
	puzzleCursor     = 0;
	puzzleScroll     = 0;
	showWrong        = false;

	puzzle5Mode      = false;
	puzzle5Minimized = false;
	puzzle5Text.clear();
	puzzle5Cursor    = 0;
	puzzle5Scroll    = 0;
	puzzle5Wrong     = false;

	puzzle6Mode      = false;
	puzzle6Minimized = false;
	puzzle6Text.clear();
	puzzle6Cursor    = 0;
	puzzle6Scroll    = 0;
	puzzle6Wrong     = false;
	puzzle6Col       = -1;
	

	while (running) {
		// ------------------------Event Handling -------------------------
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				// Player clicked the window close button: exit loop
				running = false;
			}

			// -------------- Puzzle 6 Overlay Input Handling ----------------------------------
			// If we are currently viewing Puzzle 6 and it is not minimized,
			// handle keystrokes: left/right arrow to move cursor, Escape to
			// minimize, Backspace to delete, Enter to submit answer, and
			// TEXTINPUT to insert typed characters.
			if (currentPuzzleIdx == 5 && !puzzle6Minimized) {
				if (e.type == SDL_KEYDOWN) {
					if (e.key.keysym.sym == SDLK_LEFT) {
						if (puzzle6Cursor > 0) puzzle6Cursor--;
					}
					else if (e.key.keysym.sym == SDLK_RIGHT) {
						if (puzzle6Cursor < (int)puzzle6Text.size()) puzzle6Cursor++;
					}
					else if (e.key.keysym.sym == SDLK_ESCAPE) {
						puzzle6Minimized = true;
						SDL_StopTextInput();
						continue; // Skip to next event
					}
					else if (e.key.keysym.sym == SDLK_BACKSPACE) {
						if (puzzle6Cursor > 0) {
							puzzle6Text.erase(puzzle6Cursor - 1, 1);
							puzzle6Cursor--;
						}
					}
					else if (e.key.keysym.sym == SDLK_RETURN) {
						// Compare lowercased input to correct answer
						string u = puzzle6Text;
						string a = puzzleSets[chosenSetIndex][5].answer;
						for (auto &c : u) c = tolower(c);
						for (auto &c : a) c = tolower(c);
						if (u == a) {
							// Correct answer actions
							puzzle6Solved     = true;
							puzzle6Mode       = false;
							puzzle6Minimized  = false;
							currentPuzzleIdx  = -1;
							puzzle6Text.clear();
							puzzle6Cursor     = 0;
							puzzle6Scroll     = 0;
							puzzle6Wrong      = false;
							SDL_StopTextInput();

							// Open all doors on row 0
							for (int c = 0; c < MAP_COLS; c++) {
								if (tilemap[0][c] == DOOR) {
									tilemap[0][c] = OPENDOOR;
								}
							}
						} else {
							puzzle6Wrong = true;
						}
					}
				}
				else if (e.type == SDL_TEXTINPUT) {
					puzzle6Text.insert(puzzle6Cursor, e.text.text);
					puzzle6Cursor += (int)strlen(e.text.text);
				}
				continue; // Skip further handling while Puzzle 6 overlay is active
			}


			// ---------------- Puzzle 5 Overlay Input Handling -----------------------
			// Similar logic for Puzzle 5: left/right arrow to move cursor,
			// Escape to minimize, Backspace, Enter to submit, TEXTINPUT to insert.
			if (currentPuzzleIdx == 4 && !puzzle5Minimized) {
				if (e.type == SDL_KEYDOWN) {
					if (e.key.keysym.sym == SDLK_LEFT) {
						if (puzzle5Cursor > 0) puzzle5Cursor--;
					}
					else if (e.key.keysym.sym == SDLK_RIGHT) {
						if (puzzle5Cursor < (int)puzzle5Text.size()) puzzle5Cursor++;
					}
					else if (e.key.keysym.sym == SDLK_ESCAPE) {
						puzzle5Minimized = true;
						SDL_StopTextInput();
						continue;
					}
					else if (e.key.keysym.sym == SDLK_BACKSPACE) {
						if (puzzle5Cursor > 0) {
							puzzle5Text.erase(puzzle5Cursor - 1, 1);
							puzzle5Cursor--;
						}
					}
					else if (e.key.keysym.sym == SDLK_RETURN) {
						// Compare lowercased input to correct answer for Puzzle 5
						string u = puzzle5Text;
						string a = puzzleSets[chosenSetIndex][4].answer;
						for (auto &c : u) c = tolower(c);
						for (auto &c : a) c = tolower(c);
						if (u == a) {
							// Correct answer logic
							puzzle5Solved     = true;
							puzzle5Mode       = false;
							puzzle5Minimized  = false;
							currentPuzzleIdx  = -1;
							puzzle5Text.clear();
							puzzle5Cursor     = 0;
							puzzle5Scroll     = 0;
							puzzle5Wrong      = false;
							SDL_StopTextInput();

							// Remove the two new statues on row 1 and replace with FLOOR
							tilemap[1][NEW_LEFT_COL]  = FLOOR;
							tilemap[1][NEW_RIGHT_COL] = FLOOR;
							// Convert all LASER tiles on row 0 to DOOR
							for (int c = 0; c < MAP_COLS; c++) {
								if (tilemap[0][c] == LASER) {
									tilemap[0][c] = DOOR;
								}
							}
						} else {
							puzzle5Wrong = true;
						}
					}
				}
				else if (e.type == SDL_TEXTINPUT) {
					puzzle5Text.insert(puzzle5Cursor, e.text.text);
					puzzle5Cursor += (int)strlen(e.text.text);
				}
				continue; // Skip further handling while Puzzle 5 is active
			}


			//  Puzzle 1-4 Overlay Input Handling
			// If we are viewing one of puzzles 1-4 (currentPuzzleIdx in 0..3) and
			// it is not minimized, handle similar input: arrow keys, Escape,
			// Backspace, Enter, TEXTINPUT.
			if (currentPuzzleIdx >= 0 && currentPuzzleIdx < 4 && !puzzleMinimized) {
				if (e.type == SDL_KEYDOWN) {
					SDL_Keycode key = e.key.keysym.sym;
					if (key == SDLK_LEFT) {
						if (puzzleCursor > 0) puzzleCursor--;
					}
					else if (key == SDLK_RIGHT) {
						if (puzzleCursor < (int)puzzleText.size()) puzzleCursor++;
					}
					else if (key == SDLK_ESCAPE) {
						puzzleMinimized = true;
						SDL_StopTextInput();
						continue;
					}
					else if (key == SDLK_BACKSPACE) {
						if (puzzleCursor > 0) {
							puzzleText.erase(puzzleCursor - 1, 1);
							puzzleCursor--;
						}
					}
					else if (key == SDLK_RETURN) {
						// Compare the typed answer (lowercased) to the correct one
						string u = puzzleText;
						string a = puzzles[currentPuzzleIdx].answer;
						for (auto &c : u) c = tolower(c);
						for (auto &c : a) c = tolower(c);
						if (u == a) {
							// Correct: mark puzzle solved, close overlay.
							puzzles[currentPuzzleIdx].solved = true;
							puzzleMode       = false;
							puzzleMinimized  = false;
							currentPuzzleIdx = -1;
							puzzleText.clear();
							puzzleCursor     = 0;
							puzzleScroll     = 0;
							showWrong        = false;
							SDL_StopTextInput();

							// If all four puzzles (1-4) are solved now, convert row 1 lasers
							int solvedCount = 0;
							for (auto &pz : puzzles) {
								if (pz.solved) solvedCount++;
							}
							if (solvedCount == 4 && !insertedNewStatues) {
								for (int c = 0; c < MAP_COLS; c++) {
									if (tilemap[1][c] == LASER) {
										if (c == NEW_LEFT_COL || c == NEW_RIGHT_COL) {
											tilemap[1][c] = NEWSTATUE;
										} else {
											tilemap[1][c] = FLOOR;
										}
									}
								}
								insertedNewStatues = true;
							}
						} else {
							// Wrong answer: set flag to show Wrong answer!
							showWrong = true;
						}
					}
				}
				else if (e.type == SDL_TEXTINPUT) {
					puzzleText.insert(puzzleCursor, e.text.text);
					puzzleCursor += (int)strlen(e.text.text);
				}
				continue;  // Skip further handling while puzzles 1-4 are active
			}

			// Gameplay & Puzzle Activation (No Overlay/Minimized)
			// If the player is not dead and no puzzle overlay is blocking input,
			// handle movement (WASD/arrow), and pressing Enter to activate puzzles
			// or queue fireballs.


			if (!playerDead &&
			        (currentPuzzleIdx < 0 || puzzleMinimized ||
			         (currentPuzzleIdx == 4 && puzzle5Minimized) ||
			         (currentPuzzleIdx == 5 && puzzle6Minimized)))
			{
				if (e.type == SDL_KEYDOWN) {
					int newX = playerX;
					int newY = playerY;
					switch (e.key.keysym.sym) {
					case SDLK_UP:
						newY--;
						break;
					case SDLK_w:
						newY--;
						break;
					case SDLK_DOWN:
						newY++;
						break;
					case SDLK_s:
						newY++;
						break;
					case SDLK_LEFT:
						newX--;
						break;
					case SDLK_a:
						newX--;
						break;
					case SDLK_RIGHT:
						newX++;
						break;
					case SDLK_d:
						newX++;
						break;

					case SDLK_RETURN: {
						// If Enter is pressed, attempt to activate a puzzle or
						// queue fireballs if it---s a wrong statue or wrong switch.
						// Only proceed if no fireballs are currently in motion
						// and none are queued.
						if (!fireballsActive && !fireballQueued) {
							// --- Puzzle 6 activation: stand at row 1 under a DOOR ---
							if (puzzle5Solved && !puzzle6Solved &&
							        playerY == 1 && tilemap[0][playerX] == DOOR)
							{
								// Only allow Puzzle 6 if all stones are collected
								if (collectedCount == totalStones) {
									puzzle6Mode       = true;
									puzzle6Minimized  = false;
									puzzle6Text.clear();
									puzzle6Cursor     = 0;
									puzzle6Scroll     = 0;
									puzzle6Wrong      = false;
									currentPuzzleIdx  = 5;
									puzzle6Col        = playerX;
									SDL_StartTextInput();
								}
								// If not all stones are collected, do nothing
							}
							// --- Puzzle 5 activation: stand at row 2 under NEWSTATUE ---
							else if (insertedNewStatues && !puzzle5Solved &&
							         playerY == 2 &&
							         (playerX == NEW_LEFT_COL || playerX == NEW_RIGHT_COL))
							{
								// If on the left NEWSTATUE, open Puzzle 5 overlay.
								// If on the right, its a wrong switch  queue fireballs.
								if (playerX == NEW_LEFT_COL) {
									puzzle5Mode       = true;
									puzzle5Minimized  = false;
									puzzle5Text.clear();
									puzzle5Cursor     = 0;
									puzzle5Scroll     = 0;
									puzzle5Wrong      = false;
									currentPuzzleIdx  = 4;
									SDL_StartTextInput();
								} else {
									// Wrong switch: schedule fireballs immediately
									fireballQueued     = true;
									fireballTriggerTime= SDL_GetTicks();
								}
							}
							// --- Puzzles 1-4 activation: stand below a statue ---
							else {
								int statueY = playerY - 1;
								int statueX = playerX;
								if (statueY >= 0 && tilemap[statueY][statueX] == STATUE) {
									int idx = correctStatue(statueY, statueX);
									if (idx >= 0) {
										// If this statue is one of the correct switches
										if (!puzzles[idx].solved) {
											puzzleMode       = true;
											puzzleMinimized  = false;
											showWrong        = false;
											puzzleText.clear();
											puzzleCursor     = 0;
											puzzleScroll     = 0;
											currentPuzzleIdx = idx;
											SDL_StartTextInput();
										}
									} else {
										// Wrong statue: queue fireballs
										fireballQueued     = true;
										fireballTriggerTime= SDL_GetTicks();
									}
								}
							}
						}
						break;
					}
					}
					// After keydown, attempt to move the player to (newX,newY) if itb  s
					// within bounds and is a floor tile. Also check for stone collection.
					if (newX >= 0 && newX < MAP_COLS && newY >= 0 && newY < MAP_ROWS) {
						if (tilemap[newY][newX] == FLOOR) {
							playerX = newX;
							playerY = newY;
							// Check if we just stepped on a stone
							for (int i = 0; i < totalStones; i++) {
								if (!stoneCollected[i] &&
								        stonePositions[i].first == playerY &&
								        stonePositions[i].second == playerX)
								{
									stoneCollected[i] = true;
									collectedCount++;
									break;
								}
							}
						}
					}
				}
			}
		}

		//                                                                                                 Delayed Fireball Spawn                                                                                                                              
		// If we queued fireballs and the designated delay has passed, spawn them.
		if (fireballQueued && SDL_GetTicks() - fireballTriggerTime >= FIREBALL_DELAY) {
			spawnFireballs();
			fireballQueued = false;
		}

		//                                                                                                 Move Fireballs & Handle Collisions                                                                  
		Uint32 curTick = SDL_GetTicks();
		float delta    = (curTick - lastTick) / 1000.0f;  // Delta time in seconds
		lastTick       = curTick;

		if (fireballsActive) {
			// Move each fireball horizontally
			for (auto &f : fireballs) {
				if (!f.active) continue;
				f.x += f.dir * FIREBALL_SPEED * delta;
				// If it crosses the opposite wall, deactivate it
				if ((f.dir == +1 && f.x > MAP_COLS - 1) ||
				        (f.dir == -1 && f.x < 0))
				{
					f.active = false;
				}
			}
			// If the player is alive, check if any active fireball intersects them
			if (!playerDead) {
				for (auto &f : fireballs) {
					if (!f.active) continue;
					int fx = int(f.x + 0.5f);  // Round to nearest tile column
					if (fx == playerX && f.row == playerY) {
						// Fireball hit player: mark dead and record previous position
						playerDead   = true;
						playerPrevX  = playerX;
						playerPrevY  = playerY;
						break;
					}
				}
			}
			// Once all fireballs have gone off screen, deactivate the wave
			if (fireballsDone()) {
				fireballsActive = false;
				if (playerDead) {
					// Respawn player at previous safe location
					respawn(playerX, playerY);
				}
			}
		}

		//                                                                                                                                              Rendering                                                                                                                                                       
		SDL_RenderClear(ren);

		// Draw the entire tilemap row by row, column by column
		for (int y = 0; y < MAP_ROWS; y++) {
			for (int x = 0; x < MAP_COLS; x++) {
				SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
				SDL_Texture* tex = nullptr;

				int t = tilemap[y][x];
				if (t == WALL) {
					tex = texWall;
				}
				else if (t == LASER) {
					tex = texLaser;
				}
				else if (t == STATUE) {
					tex = texStatue;
				}
				else if (t == NEWSTATUE) {
					tex = texNewStatue;
				}
				else if (t == FLOOR) {
					tex = texTile;
				}
				else if (t == DOOR) {
					// If itb  s a closed door on row 0, we pick left vs. right door
					if (y == 0 && x == NEW_LEFT_COL) {
						tex = texDoorLeft;
					} else if (y == 0 && x == NEW_RIGHT_COL) {
						tex = texDoorRight;
					} else {
						// Fallback for any other DOOR
						tex = texDoorLeft;
					}
				}
				else if (t == OPENDOOR) {
					// Once unlocked, both doors use the same b  opendoorb   texture
					tex = texOpenDoor;
				}

				if (tex) {
					SDL_RenderCopy(ren, tex, nullptr, &dst);
				}
			}
		}


		//                           Draw Stones                                                                                                                                                                                                                                                                
		// Each stone is 20C20 pixels, centered in its 64C64 tile
		for (int i = 0; i < totalStones; i++) {
			if (!stoneCollected[i]) {
				int sy = stonePositions[i].first;   // stone row
				int sx = stonePositions[i].second;  // stone column
				// Center 20C20 within the 64C64 tile
				int px = sx * TILE_SIZE + (TILE_SIZE - 20) / 2;
				int py = sy * TILE_SIZE + (TILE_SIZE - 20) / 2;
				SDL_Rect sdst = { px, py, 20, 20 };
				SDL_RenderCopy(ren, texStone, nullptr, &sdst);
			}
		}

		//                           Draw Fireballs                                                                                                                                                                                                                                            
		if (fireballsActive) {
			for (auto &f : fireballs) {
				if (!f.active) continue;
				SDL_Rect fb = { int(f.x * TILE_SIZE), f.row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
				SDL_RenderCopy(ren, texFireball, nullptr, &fb);
			}
		}

		//                           Draw Player                                                                                                                                                                                                                                                                
		// Only draw the player if they are alive and no puzzle overlay blocks them
		bool overlayBlocksPlayer =
		    (currentPuzzleIdx >= 0 && currentPuzzleIdx < 4 && !puzzleMinimized) ||
		    (currentPuzzleIdx == 4 && !puzzle5Minimized) ||
		    (currentPuzzleIdx == 5 && !puzzle6Minimized);
		if (!playerDead && !overlayBlocksPlayer) {
			SDL_Rect pr = { playerX * TILE_SIZE, playerY * TILE_SIZE, TILE_SIZE, TILE_SIZE };
			SDL_RenderCopy(ren, texGengar, nullptr, &pr);
		}

		//                           Draw Puzzle 1b  4 Overlay                                                                                                                                                                                                    
		// If one of puzzles 1b  4 is active (currentPuzzleIdx 0..3) and not minimized,
		// draw a semi transparent black rectangle over the whole screen, then
		// draw the puzzle image at the top 2/3 of the screen, and the input box below.
		if (currentPuzzleIdx >= 0 && currentPuzzleIdx < 4 && !puzzleMinimized) {
			// Dim background
			SDL_SetRenderDrawColor(ren, 0, 0, 0, 192);
			SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
			SDL_RenderFillRect(ren, &full);

			// Draw the puzzle image (scaled to top 2/3 of window)
			int w = SCREEN_WIDTH;
			int h = (SCREEN_HEIGHT * 2) / 3;
			SDL_Rect dst = {0, 0, w, h};
			SDL_RenderCopy(ren, puzzles[currentPuzzleIdx].image, nullptr, &dst);

			// Draw the input box below (a simple gray rectangle)
			int boxX = 50;
			int boxY = h + 30;
			int boxW = SCREEN_WIDTH - 100;
			int boxH = 60;
			SDL_SetRenderDrawColor(ren, 32, 32, 32, 255);
			SDL_Rect ibox = { boxX, boxY, boxW, boxH };
			SDL_RenderFillRect(ren, &ibox);

			// Determine pixel position of the cursor (width of substring)
			int cursorPixelX = 0;
			if (!puzzleText.empty()) {
				string leftSub = puzzleText.substr(0, puzzleCursor);
				TTF_SizeUTF8(font, leftSub.c_str(), &cursorPixelX, nullptr);
			}
			// If the cursor has moved beyond visible area, scroll right
			if (cursorPixelX - puzzleScroll > boxW - 20) {
				puzzleScroll = cursorPixelX - (boxW - 20);
			}
			// If the cursor is left of the visible window, scroll left
			if (cursorPixelX < puzzleScroll) {
				puzzleScroll = cursorPixelX;
			}

			// Render the entire typed text to a texture, then draw only the visible part
			SDL_Color white = {255, 255, 255};
			SDL_Texture* textTex = renderText(ren, puzzleText, white);
			if (textTex) {
				int texW, texH;
				SDL_QueryTexture(textTex, nullptr, nullptr, &texW, &texH);
				// Compute how many pixels remain to display from puzzleScroll
				int available = texW - puzzleScroll;
				if (available < 0) available = 0;
				int visibleW =  min(available, boxW - 20);

				SDL_Rect src  = { puzzleScroll, 0, visibleW, texH };
				SDL_Rect dst2 = { boxX + 10, boxY + (boxH - texH) / 2, visibleW, texH };
				SDL_RenderCopy(ren, textTex, &src, &dst2);
				SDL_DestroyTexture(textTex);
			}

			// If the last answer attempt was wrong, draw b  Wrong answer!b   in red
			if (showWrong) {
				SDL_Color red = {255, 60, 60};
				SDL_Texture* wt = renderText(ren, "Wrong answer!", red);
				if (wt) {
					int tw, th;
					SDL_QueryTexture(wt, nullptr, nullptr, &tw, &th);
					SDL_Rect tr = { (SCREEN_WIDTH - tw) / 2, boxY + boxH + 10, tw, th };
					SDL_RenderCopy(ren, wt, nullptr, &tr);
					SDL_DestroyTexture(wt);
				}
			}
		}

		//                           Draw Puzzle 5 Overlay                                                                                                                                                                                                              
		// If Puzzle 5 (index 4) is active and not minimized, draw similarly:
		if (currentPuzzleIdx == 4 && !puzzle5Minimized) {
			// Dim background
			SDL_SetRenderDrawColor(ren, 0, 0, 0, 192);
			SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
			SDL_RenderFillRect(ren, &full);

			// Draw Puzzle 5 image in top 2/3
			int w = SCREEN_WIDTH;
			int h = (SCREEN_HEIGHT * 2) / 3;
			SDL_Rect dst = {0, 0, w, h};
			SDL_RenderCopy(ren, puzzleSets[chosenSetIndex][4].image, nullptr, &dst);

			// Draw input box below
			int boxX = 50;
			int boxY = h + 30;
			int boxW = SCREEN_WIDTH - 100;
			int boxH = 60;
			SDL_SetRenderDrawColor(ren, 32, 32, 32, 255);
			SDL_Rect ibox = { boxX, boxY, boxW, boxH };
			SDL_RenderFillRect(ren, &ibox);

			// Calculate cursor pixel X for puzzle5Text
			int cursorPixelX = 0;
			if (!puzzle5Text.empty()) {
				string leftSub = puzzle5Text.substr(0, puzzle5Cursor);
				TTF_SizeUTF8(font, leftSub.c_str(), &cursorPixelX, nullptr);
			}
			// Scroll if necessary
			if (cursorPixelX - puzzle5Scroll > boxW - 20) {
				puzzle5Scroll = cursorPixelX - (boxW - 20);
			}
			if (cursorPixelX < puzzle5Scroll) {
				puzzle5Scroll = cursorPixelX;
			}

			// Render the typed text for Puzzle 5, draw visible slice
			SDL_Color white = {255, 255, 255};
			SDL_Texture* textTex5 = renderText(ren, puzzle5Text, white);
			if (textTex5) {
				int texW, texH;
				SDL_QueryTexture(textTex5, nullptr, nullptr, &texW, &texH);
				int available = texW - puzzle5Scroll;
				if (available < 0) available = 0;
				int visibleW =  min(available, boxW - 20);

				SDL_Rect src  = { puzzle5Scroll, 0, visibleW, texH };
				SDL_Rect dst2 = { boxX + 10, boxY + (boxH - texH) / 2, visibleW, texH };
				SDL_RenderCopy(ren, textTex5, &src, &dst2);
				SDL_DestroyTexture(textTex5);
			}

			// If Puzzle 5 answer was wrong, show b  Wrong answer!b  
			if (puzzle5Wrong) {
				SDL_Color red = {255, 60, 60};
				SDL_Texture* wt = renderText(ren, "Wrong answer!", red);
				if (wt) {
					int tw, th;
					SDL_QueryTexture(wt, nullptr, nullptr, &tw, &th);
					SDL_Rect tr = { (SCREEN_WIDTH - tw) / 2, boxY + boxH + 10, tw, th };
					SDL_RenderCopy(ren, wt, nullptr, &tr);
					SDL_DestroyTexture(wt);
				}
			}
		}

		//                           Draw Puzzle 6 Overlay                                                                                                                                                                                                              
		// If Puzzle 6 (index 5) is active and not minimized, do the same pattern.
		if (currentPuzzleIdx == 5 && !puzzle6Minimized) {
			// Dim background
			SDL_SetRenderDrawColor(ren, 0, 0, 0, 192);
			SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
			SDL_RenderFillRect(ren, &full);

			// Draw Puzzle 6 image in top 2/3
			int w = SCREEN_WIDTH;
			int h = (SCREEN_HEIGHT * 2) / 3;
			SDL_Rect dst = {0, 0, w, h};
			SDL_RenderCopy(ren, puzzleSets[chosenSetIndex][5].image, nullptr, &dst);

			// Draw input box below
			int boxX = 50;
			int boxY = h + 30;
			int boxW = SCREEN_WIDTH - 100;
			int boxH = 60;
			SDL_SetRenderDrawColor(ren, 32, 32, 32, 255);
			SDL_Rect ibox = { boxX, boxY, boxW, boxH };
			SDL_RenderFillRect(ren, &ibox);

			// Compute cursor pixel X for puzzle6Text
			int cursorPixelX = 0;
			if (!puzzle6Text.empty()) {
				string leftSub = puzzle6Text.substr(0, puzzle6Cursor);
				TTF_SizeUTF8(font, leftSub.c_str(), &cursorPixelX, nullptr);
			}
			if (cursorPixelX - puzzle6Scroll > boxW - 20) {
				puzzle6Scroll = cursorPixelX - (boxW - 20);
			}
			if (cursorPixelX < puzzle6Scroll) {
				puzzle6Scroll = cursorPixelX;
			}

			// Render typed text as texture and draw only visible slice
			SDL_Color white = {255, 255, 255};
			SDL_Texture* textTex6 = renderText(ren, puzzle6Text, white);
			if (textTex6) {
				int texW, texH;
				SDL_QueryTexture(textTex6, nullptr, nullptr, &texW, &texH);
				int available = texW - puzzle6Scroll;
				if (available < 0) available = 0;
				int visibleW =  min(available, boxW - 20);

				SDL_Rect src  = { puzzle6Scroll, 0, visibleW, texH };
				SDL_Rect dst2 = { boxX + 10, boxY + (boxH - texH) / 2, visibleW, texH };
				SDL_RenderCopy(ren, textTex6, &src, &dst2);
				SDL_DestroyTexture(textTex6);
			}

			// If Puzzle 6 answer was wrong, show b  Wrong answer!b  
			if (puzzle6Wrong) {
				SDL_Color red = {255, 60, 60};
				SDL_Texture* wt = renderText(ren, "Wrong answer!", red);
				if (wt) {
					int tw, th;
					SDL_QueryTexture(wt, nullptr, nullptr, &tw, &th);
					SDL_Rect tr = { (SCREEN_WIDTH - tw) / 2, boxY + boxH + 10, tw, th };
					SDL_RenderCopy(ren, wt, nullptr, &tr);
					SDL_DestroyTexture(wt);
				}
			}
		}

		const Uint8* keys = SDL_GetKeyboardState(NULL);
		if (puzzle6Solved &&
		        playerY == 1 && (playerX == NEW_LEFT_COL || playerX == NEW_RIGHT_COL))
		{
			// Only register a single key press
			static bool enterPressedLast = false;
			if (keys[SDL_SCANCODE_RETURN]) {
				if (!enterPressedLast) {
					readyToExit = true;
					running = false;
				}
				enterPressedLast = true;
			} else {
				enterPressedLast = false;
			}
		}

		// Present everything and frame delay (as in your code)


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
			SDL_Texture* timerTex = SDL_CreateTextureFromSurface(ren, surf);
			SDL_Rect timerRect = {20, 20, surf->w, surf->h};

			SDL_RenderCopy(ren, timerTex, nullptr, &timerRect);

			SDL_FreeSurface(surf);
			SDL_DestroyTexture(timerTex);
			TTF_CloseFont(fontTimer);
		}

		SDL_RenderPresent(ren);
		SDL_Delay(16);
	}

	
	SDL_DestroyTexture(texWall);
	SDL_DestroyTexture(texLaser);
	SDL_DestroyTexture(texStatue);
	SDL_DestroyTexture(texTile);
	SDL_DestroyTexture(texGengar);
	SDL_DestroyTexture(texFireball);
	SDL_DestroyTexture(texNewStatue);
	SDL_DestroyTexture(texDoorLeft);
	SDL_DestroyTexture(texDoorRight);
	SDL_DestroyTexture(texOpenDoor);
	SDL_DestroyTexture(texStone);
	for (int s = 0; s < NUM_SETS; s++)
		for (int p = 0; p < PUZZLES_PER_SET; p++)
			SDL_DestroyTexture(puzzleSetsImgArr[s][p]);
	TTF_CloseFont(font);


	
	return readyToExit;
}

