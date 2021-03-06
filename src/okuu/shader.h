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

#define SHADER_FILE 0
#define SHADER_SRC  1

typedef char layout_name_t[256];

typedef struct {
    const char* name;
    int layout_cnt;
    layout_name_t* layout;

    GLuint program;
    GLint* attribs;
    int loaded;
} shader_t;

shader_t* shader_create(const char*);

int shader_layout(shader_t*, int, ...);
int shader_source(shader_t*, int, int, ...);
int shader_attribs(shader_t*, int, ...);
GLint shader_attr(const shader_t*, int);

shader_t* shader_active();
void shader_start(shader_t*);
void shader_stop();

void shader_destroy(shader_t*);

#endif
