#ifndef OKUU_TERRAIN_H
#define OKUU_TERRAIN_H

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <cglm/cglm.h>
#include <GL/glu.h>

#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "koa/file.h"

#define CHUNK_SIZE 32
#define TERR_BUFFS 3

typedef struct {
    bmp_t* bmps[2];
    int center_x, center_y;
    float heights[CHUNK_SIZE][CHUNK_SIZE];
    GLuint buffers[TERR_BUFFS], vao;
} terrain_t;

terrain_t* terrain_load(const char*, const char*, int, int);
void terrain_move(terrain_t*, int, int);
void terrain_unload(terrain_t*);

#endif
