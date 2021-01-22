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

#define CHUNK_SIZE 20
#define TERR_BUFFS 4

typedef struct {
    bmp_t* bmps[2];
    color_t* paths;
    int top_x, top_y, width, height,
        tri_cnt, path_cnt;
    float heights[CHUNK_SIZE][CHUNK_SIZE];
    GLuint buffers[TERR_BUFFS], vao;

    float pick_dist;
    int pick_depth;
} terrain_t;

terrain_t* terrain_load
    (const char*, const char*, const color_t*, int, int, int);
void terrain_move(terrain_t*, int, int);
float terrain_height(terrain_t*, int, int);
void terrain_pick_params(terrain_t*, float, int);
int terrain_pick(terrain_t*, vec3, vec3, vec3);
void terrain_unload(terrain_t*);

#endif
