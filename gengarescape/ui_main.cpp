//g++ -std=c++17 music.cpp main_map.cpp ui_main.cpp trap1.cpp trap2.cpp trap3.cpp trap4.cpp -o prog -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -ldl


//header includes

#include<bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include "trap4.hpp"
#include "trap3.hpp"
#include "trap2.hpp"
#include "trap1.hpp"
#include "header.hpp"
#include "music.h"


using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* fontui = nullptr;
SDL_Texture* backImageTexture = nullptr;
string playerName;

//constants
int SCREEN_WIDTH = 1152;
int SCREEN_HEIGHT = 768;
int POPUP_SIZE = 550;
int buttonWidth = 200;
int buttonHeight = 45;
int PopUpX = 297;
int PopUpY = 150;

//states
int currState = 0;
const int STATE_NAME_INPUT = 100;
const int STATE_LEADERBOARD = 7;

//spacing
int verticalSpacing = 100;
int horizontalSpacing = 325;
int verticalSpacing2 = 20;

//pop up
SDL_Rect PopUpRect{PopUpX, PopUpY, POPUP_SIZE, POPUP_SIZE};
bool PopUpShow = true;
//prototype
void addLeaderboardEntry(const string& name, int timeSeconds);

//structures
struct LeaderboardEntry {
    string name;
    int timeSeconds;  // Elapsed time in seconds

    // For sorting (less time is better)
    bool operator<(const LeaderboardEntry& other) const {
        return timeSeconds < other.timeSeconds;
    }
};



struct Buttons {
    SDL_Rect button;
    string text;
    bool visible = true;
};

//vectors
vector<LeaderboardEntry> leaderboard;
vector<Buttons> MainButtons =
{
    {{200, 400, buttonWidth, buttonHeight}, "Play", true},
    {{200 + buttonWidth + horizontalSpacing, 400, buttonWidth, buttonHeight}, "Leaderboard", true},
    {{200, 400 + buttonHeight + verticalSpacing, buttonWidth, buttonHeight}, "Settings", true},
    {{200 + buttonWidth + horizontalSpacing, 400 + buttonHeight + verticalSpacing, buttonWidth, buttonHeight}, "About", true}
};

vector<Buttons> SettingButtons = {
    {{476, 300, buttonWidth, buttonHeight}, "Controls"},
    {{476,400, buttonWidth, buttonHeight}, "Goal"},
    {{476, 500, buttonWidth, buttonHeight}, "Sound"}
};

// New global variable to store the back button texture and rect
SDL_Rect backButtonRect = {302, 155, 40, 40};

//functions
bool loadBackButtonImage() {
    backImageTexture = IMG_LoadTexture(renderer, "back.png");  // Load your back.png
    return true;
}

void renderBackButton() {
    if (backImageTexture) {
        SDL_RenderCopy(renderer, backImageTexture, NULL, &backButtonRect);
    }
}

void clearVis(vector<Buttons>& b) {
    for (int i = 0; i < b.size(); i++)
        b[i].visible = false;
}

void renderButtons(vector<Buttons>& b, SDL_Point mouse) {
    SDL_Color white = {255, 255, 255, 255};  // Text color (white)
    for (int i = 0; i < b.size(); i++) {
        if (b[i].visible) {
            bool isHovered = SDL_PointInRect(&mouse, &b[i].button);
            SDL_Color buttonColor = {80, 80, 80, SDL_ALPHA_OPAQUE};
            if (isHovered) {
                buttonColor = {60, 60, 60, SDL_ALPHA_OPAQUE};
            }
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(renderer, &b[i].button);

            SDL_Surface* surface = TTF_RenderText_Blended(fontui, b[i].text.c_str(), white);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

            int textWidth = surface->w;
            int textHeight = surface->h;
            SDL_Rect dstRect = {
                b[i].button.x + (b[i].button.w - textWidth) / 2,
                b[i].button.y + (b[i].button.h - textHeight) / 2,
                textWidth,
                textHeight
            };

            SDL_RenderCopy(renderer, texture, NULL, &dstRect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
    }
}

void RenderPopUp() {
    if (PopUpShow) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, &PopUpRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void transitionToGame(SDL_Window* window, SDL_Renderer* renderer) {
    GameResult result = runGame(window, renderer);
    if (result.won) {
        addLeaderboardEntry(playerName, result.winTimeSeconds);
    }
}

string showNameInput(SDL_Window* window, SDL_Renderer* renderer) {
    bool done = false;
    string input;
    SDL_StartTextInput();

    TTF_Font* fontname = TTF_OpenFont("font.ttf", 48);
    SDL_Color textColor = {255, 255, 255};

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                exit(0);

            if (event.type == SDL_TEXTINPUT) {
                input += event.text.text;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !input.empty())
                    input.pop_back();
                if (event.key.keysym.sym == SDLK_RETURN && !input.empty())
                    done = true;
            }
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
        SDL_RenderFillRect(renderer, NULL);

        string prompt = "Enter your name:";
        SDL_Surface* surfPrompt = TTF_RenderText_Solid(fontname, prompt.c_str(), textColor);
        SDL_Texture* texPrompt = SDL_CreateTextureFromSurface(renderer, surfPrompt);

        int winW = 800, winH = 800;
        SDL_GetWindowSize(window, &winW, &winH);

        SDL_Rect promptRect = { (winW - surfPrompt->w)/2, (winH - surfPrompt->h)/2 - 60, surfPrompt->w, surfPrompt->h };
        SDL_RenderCopy(renderer, texPrompt, nullptr, &promptRect);

        SDL_FreeSurface(surfPrompt);
        SDL_DestroyTexture(texPrompt);

        // Draw input text or a placeholder line if empty
        if (!input.empty()) {
            SDL_Surface* surfInput = TTF_RenderText_Solid(fontname, input.c_str(), textColor);
            if (!surfInput) {
                cerr << "[showNameInput] TTF_RenderText_Solid error (input): " << TTF_GetError() << std::endl;
                exit(1);
            }
            SDL_Texture* texInput = SDL_CreateTextureFromSurface(renderer, surfInput);
            if (!texInput) {
                cerr << "[showNameInput] SDL_CreateTextureFromSurface error (input): " << SDL_GetError() << std::endl;
                SDL_FreeSurface(surfInput);
                exit(1);
            }
            SDL_Rect inputRect = { (winW - surfInput->w)/2, (winH - surfInput->h)/2 + 10, surfInput->w, surfInput->h };
            SDL_RenderCopy(renderer, texInput, nullptr, &inputRect);
            SDL_FreeSurface(surfInput);
            SDL_DestroyTexture(texInput);
        } else {
            SDL_Rect placeholder = { winW/2 - 200, winH/2 + 34, 400, 3 };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
            SDL_RenderFillRect(renderer, &placeholder);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    TTF_CloseFont(fontname);
    SDL_StopTextInput();
    return input;
}

void loadLeaderboard(const string& filename) {
    leaderboard.clear();
    ifstream fin(filename);
    if (!fin) return; // no file yet, fine
    string name;
    int time;
    while (fin >> name >> time) {
        leaderboard.push_back({name, time});
    }
    fin.close();
    sort(leaderboard.begin(), leaderboard.end());
}

// --- Save leaderboard to file ---
void saveLeaderboard(const string& filename) {
    ofstream fout(filename);
    for (const auto& entry : leaderboard) {
        fout << entry.name << " " << entry.timeSeconds << "\n";
    }
    fout.close();
}

// --- Add a new entry and keep best 10 ---
void addLeaderboardEntry(const string& name, int timeSeconds) {
    leaderboard.push_back({name, timeSeconds});
    sort(leaderboard.begin(), leaderboard.end());
    if (leaderboard.size() > 10) leaderboard.resize(10); // Top 10 only
    saveLeaderboard("leaderboard.txt");
}
 



// --- Render leaderboard popup ---
void renderLeaderboardPopup(SDL_Renderer* renderer, TTF_Font* font) {
    // Popup bg
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 210);
    SDL_RenderFillRect(renderer, &PopUpRect);

    // Headers
    SDL_Color headerColor = {255, 255, 0}; // yellow-ish
    SDL_Color textColor = {255, 255, 255};
    int col1 = PopUpX + 50;
    int col2 = PopUpX + 200;
    int col3 = PopUpX + 400;

    TTF_Font* fontHeader = TTF_OpenFont("fonthealth.ttf", 32);
    TTF_Font* fontText = TTF_OpenFont("fonthealth.ttf", 28);

    SDL_Surface* sRank = TTF_RenderText_Solid(fontHeader, "Rank", headerColor);
    SDL_Surface* sName = TTF_RenderText_Solid(fontHeader, "Name", headerColor);
    SDL_Surface* sTime = TTF_RenderText_Solid(fontHeader, "Time", headerColor);

    SDL_Texture* tRank = SDL_CreateTextureFromSurface(renderer, sRank);
    SDL_Texture* tName = SDL_CreateTextureFromSurface(renderer, sName);
    SDL_Texture* tTime = SDL_CreateTextureFromSurface(renderer, sTime);

    SDL_Rect rRank = {col1, PopUpY + 50, sRank->w, sRank->h};
    SDL_Rect rName = {col2, PopUpY + 50, sName->w, sName->h};
    SDL_Rect rTime = {col3, PopUpY + 50, sTime->w, sTime->h};
    SDL_RenderCopy(renderer, tRank, nullptr, &rRank);
    SDL_RenderCopy(renderer, tName, nullptr, &rName);
    SDL_RenderCopy(renderer, tTime, nullptr, &rTime);

    SDL_FreeSurface(sRank); SDL_FreeSurface(sName); SDL_FreeSurface(sTime);
    SDL_DestroyTexture(tRank); SDL_DestroyTexture(tName); SDL_DestroyTexture(tTime);
    TTF_CloseFont(fontHeader);

    // Draw entries
    int rowY = PopUpY + 110;
    for (size_t i = 0; i < leaderboard.size(); ++i) {
        // Rank
        string srank = std::to_string(i + 1);
        SDL_Surface* s1 = TTF_RenderText_Solid(fontText, srank.c_str(), textColor);
        SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, s1);
        SDL_Rect r1 = {col1, rowY, s1->w, s1->h};
        SDL_RenderCopy(renderer, t1, nullptr, &r1);
        SDL_FreeSurface(s1); SDL_DestroyTexture(t1);

        // Name
        SDL_Surface* s2 = TTF_RenderText_Solid(fontText, leaderboard[i].name.c_str(), textColor);
        SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, s2);
        SDL_Rect r2 = {col2, rowY, s2->w, s2->h};
        SDL_RenderCopy(renderer, t2, nullptr, &r2);
        SDL_FreeSurface(s2); SDL_DestroyTexture(t2);

        // Time (mm:ss)
        int t = leaderboard[i].timeSeconds;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", t / 60, t % 60);
        SDL_Surface* s3 = TTF_RenderText_Solid(fontText, buf, textColor);
        SDL_Texture* t3 = SDL_CreateTextureFromSurface(renderer, s3);
        SDL_Rect r3 = {col3, rowY, s3->w, s3->h};
        SDL_RenderCopy(renderer, t3, nullptr, &r3);
        SDL_FreeSurface(s3); SDL_DestroyTexture(t3);

        rowY += 45;
        if (i >= 9) break;
    }
    TTF_CloseFont(fontText);

    renderBackButton(); // Back button on popup
}

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL could not be initialized: " << SDL_GetError();
    } else {
        std::cout << "SDL video system is ready to go\n";
    }

   Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // Only once in main/init
   playMusic("ui_music.mp3");


    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cout << "SDL_image could not be initialized: " << IMG_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() != 0) return 1;

    window = SDL_CreateWindow("Gengar Escape: Whisper of Forbidden memories", 20, 20, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* bgTexture = IMG_LoadTexture(renderer, "background.png");

    // Load the image for the back button
    if (!loadBackButtonImage()) {
        cerr << "Failed to load back button image!" << std::endl;
        return -1;
    }

    fontui = TTF_OpenFont("font.ttf", 24);
    loadLeaderboard("leaderboard.txt");


    bool gameIsRunning = true;
    SDL_Event event;
    SDL_Point mouse;

    while (gameIsRunning) {
        int mx, my;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameIsRunning = false;
            }
            if (event.type == SDL_MOUSEMOTION) {
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point mouse = {event.button.x, event.button.y};

                if (currState == 0) {
                    for (int i = 0; i < 4; i++) {
                        if (SDL_PointInRect(&mouse, &MainButtons[i].button)) {
                            if (i == 0) currState = STATE_NAME_INPUT; // Set state for name 
                            else if (i == 1) currState = STATE_LEADERBOARD;
                            else if (i == 2) currState = 1;
                            else if (i == 3) currState = 2;
                        }
                    }
                }

                else if (currState == 1) {
                    for (int i = 0; i < 3; i++) {
                        if (SDL_PointInRect(&mouse, &SettingButtons[i].button)) {
                            if (i == 0) currState = 4;
                            if (i == 1) currState = 5;
                            if (i == 2){
                                toggleMusic();   
                                currState = 1;
                            }
                        }
                    }
                    if (SDL_PointInRect(&mouse, &backButtonRect)) {
                        currState = 0;
                    }
                }
                else if (currState == STATE_LEADERBOARD) {
                    if (SDL_PointInRect(&mouse, &backButtonRect)) {
                        currState = 0;
                    }
                }
                else if (currState == 2) {
                    if (SDL_PointInRect(&mouse, &backButtonRect)) {
                        currState = 0;
                    }
                }
                else if (currState == 4) {
                    if (SDL_PointInRect(&mouse, &backButtonRect)) {
                        currState = 1;
                    }
                }
                else if (currState == 5) {
                    if (SDL_PointInRect(&mouse, &backButtonRect)) {
                        currState = 1;
                    }
                
                }
            }
        }

        // Handle name input state
        if (currState == STATE_NAME_INPUT) {
            playerName = showNameInput(window, renderer);
            if (!playerName.empty()) {
                transitionToGame(window, renderer);
                SDL_SetWindowSize(window, 1152, 768);
            }
            currState = 0;
            continue; // Skip drawing the menu frame this loop
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        if (currState == 0) {
            PopUpShow = false;
            renderButtons(MainButtons, mouse);
        }

        else if (currState == STATE_LEADERBOARD) {
              PopUpShow = true;
              renderLeaderboardPopup(renderer, fontui);
        }

        else if (currState == 1) {
            PopUpShow = true;
            RenderPopUp();
            SettingButtons[2].text = isMusicOn ? "Sound: ON" : "Sound: OFF";
            renderButtons(SettingButtons, mouse);
            renderBackButton();
        }
        else if (currState == 2) {
            PopUpShow = true;
            RenderPopUp();
            renderBackButton();

    TTF_Font* popupFont = TTF_OpenFont("font.ttf", 18);  // Or any size you want
    SDL_Color white = {255,255,255,255};

    string controlsText = "About text";
    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(popupFont, controlsText.c_str(), white, POPUP_SIZE - 80);  // 80 for side margin
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {PopUpX + 60, PopUpY + 80, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(popupFont);
        }
        else if (currState == 4) {
            RenderPopUp();
            renderBackButton();
    
      // Render text for Controls
    TTF_Font* popupFont = TTF_OpenFont("font.ttf", 28);  // Or any size you want
    SDL_Color white = {255,255,255,255};

    string controlsText = "Keyboard:\n--> W=up\n--> A=left\n--> S=down\n--> D=right\n-->Enter=to access door and statue\n\n Mouse:\n--> Click to shoot and access buttons";
    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(popupFont, controlsText.c_str(), white, POPUP_SIZE - 80);  // 80 for side margin
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {PopUpX + 60, PopUpY + 80, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(popupFont);

        }

        else if (currState == 5) {
            RenderPopUp();
            renderBackButton();

  TTF_Font* popupFont = TTF_OpenFont("font.ttf", 28);
    SDL_Color white = {255,255,255,255};

    std
::string goalText = "--> Finish the game in least possible time   to stay on top of the leaderboard\n\nCompulsory:\n--> Defeat Final boss\n--> Solve all four portal-connected trap rooms\n--> Collect all stones in the trap room\n\nOptional:\n--> Defeat smaller enemies to reduce total time taken ";
    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(popupFont, goalText.c_str(), white, POPUP_SIZE - 80);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {PopUpX + 60, PopUpY + 80, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(popupFont);
        }

  else if (currState == 6) {
   toggleMusic();          // This now handles both stop and restart
    currState = 1; 
}

        SDL_RenderPresent(renderer);
    }
    Mix_CloseAudio();
    SDL_DestroyTexture(backImageTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(fontui);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}



