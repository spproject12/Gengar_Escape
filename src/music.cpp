// music.cpp
#include "music.h"
#include <iostream>
using namespace std;
bool isMusicOn = true;
Mix_Music* bgm = nullptr;

void playMusic(const char* filepath) {
    if (!isMusicOn) return;

    if (bgm) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
    }

    bgm = Mix_LoadMUS(filepath);
    if (!bgm) {
        cerr << "Failed to load music: " << Mix_GetError() << std::endl;
        return;
    }

    Mix_PlayMusic(bgm, -1); // Loop indefinitely
}

void stopMusic() {
    if (bgm) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
        bgm = nullptr;
    }
}

void toggleMusic() {
    isMusicOn = !isMusicOn;
    if (!isMusicOn) {
        stopMusic();
    } else {
        // Restart music from beginning if turning back on
        playMusic("ui_music.mp3");  // Hardcoded file path or make it global if needed
    }
}



