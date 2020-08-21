#ifndef RENDERER
#define RENDERER

#include <SDL2/SDL.h>

#include "renderer.c"


void rendervline(int x, int y1, int y2, SDL_Color middle, SDL_Color * texture);

void drawscreen(void);

#endif
