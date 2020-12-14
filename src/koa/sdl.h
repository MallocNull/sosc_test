#ifndef KOA_SDL_H
#define KOA_SDL_H

#include <SDL.h>
#include <stdlib.h>

typedef struct {
    uint8_t r, g, b, a;
} pixel_t;

void get_pixel(SDL_Color*, SDL_Surface*, int x, int y);

#endif
