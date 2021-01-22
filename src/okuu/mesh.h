#ifndef OKUU_MESH_H
#define OKUU_MESH_H

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "obb.h"
#include "koa/etc.h"

typedef struct {
    GLuint buffers[3], vao;
    uint32_t tri_cnt;
    obb_t obb;
} mesh_t;

mesh_t* mesh_load(const char*);

void mesh_bind(mesh_t*);
void mesh_render(mesh_t*);
void mesh_unbind();

void mesh_unload(mesh_t*);

#endif
