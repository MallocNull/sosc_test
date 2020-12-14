#ifndef OKUU_TERRAIN_H
#define OKUU_TERRAIN_H

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <cglm/cglm.h>
#include <GL/glu.h>

#include <stdlib.h>
#include <stdarg.h>

#include "koa/file.h"

#define CHUNK_SIZE 20

typedef struct {
    bmp_meta_t meta[2];
    GLuint buffers[3], vao;
    int offset_x, offset_y;
    uint8_t heights[CHUNK_SIZE][CHUNK_SIZE];
} terrain_t;

terrain_t* terrain_load(const char*, const char*, int, int);

void terrain_unload(terrain_t*);

#endif
