#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <sstream>
#include <vector>
#include "ltexture.hpp"
#include "ltimer.hpp"
#include "entity.hpp"

#ifndef MAIN_H
#define MAIN_H

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 480;
const int BG_WIDTH = 800;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int PLANEW = 88;
const int PLANEH = 73;
const int ROCKW = 108;
const int ROCKH = 239;
const int ROCKX = 114;
const int ROCKY = 1400;
const int ROCKDOWNX = 324;
const int ROCKDOWNY = 1489;
const int ROCKGRASSX = 0;
const int ROCKGRASSY = 1757;
const int ROCKGRASSDOWNX = 264;
const int ROCKGRASSDOWNY = 986;
const int ROCKICEX = 216;
const int ROCKICEY = 1639;
const int ROCKICEDOWNX = 222;
const int ROCKICEDOWNY = 1250;
const int ROCKSNOWX = 324;
const int ROCKSNOWY = 1728;
const int ROCKSNOWDOWNX = 108;
const int ROCKSNOWDOWNY = 1757;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

#endif