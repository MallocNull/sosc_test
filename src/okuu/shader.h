#ifndef OKUU_SHADER_H
#define OKUU_SHADER_H

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/glu.h>
#include <stdarg.h>

#include "koa/file.h"

#define _SHADER shader_active()
#define _ATTR(I) shader_attr(_SHADER, I)

typedef struct {
    const char* name;
    GLuint program;
    GLint* attribs;
    int loaded;
} shader_t;

shader_t* shader_create(const char*);

int shader_source(shader_t*, int, ...);
int shader_attribs(shader_t*, int, ...);
GLint shader_attr(const shader_t*, int);

shader_t* shader_active();
void shader_start(shader_t*);
void shader_stop(shader_t*);

void shader_destroy(shader_t*);

#endif
