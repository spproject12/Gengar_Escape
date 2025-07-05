// music.h
#ifndef MUSIC_H
#define MUSIC_H

#include <SDL2/SDL_mixer.h>

extern bool isMusicOn;
extern Mix_Music* bgm;

void playMusic(const char* filepath);
void stopMusic();
void toggleMusic();

#endif

