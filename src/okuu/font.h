#ifndef OKUU_FONT_H
#define OKUU_FONT_H

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <cglm/cglm.h>
#include <GL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shader.h"
#include "koa/etc.h"

typedef struct {
    float width;
    vec2 top_left,
         bottom_right;
} glyph_t;

typedef struct {
    GLuint texture;
    uint32_t width, height,
             cell_width, cell_height;
    glyph_t glyphs[256];
} font_t;

typedef struct {
    font_t* font;
    char* text;
    vec4 color;
    uint32_t
        length, size,
        wrap, tri_cnt;
    vec2 shadow;

    mat4 trans_mat;
    GLuint vao, buffers[2];
    uint8_t vert_inv, texuv_inv;
} text_t;

void font_init_subsystem(SDL_Window*);
void font_window_changed(SDL_Window*);
void font_set_default(font_t*);
void font_deinit_subsystem();

font_t* font_load(const char*, const char*, int);
void font_bind(font_t*);
void font_unbind();
void font_unload(font_t*);

text_t* text_create(font_t*);
void text_set(text_t*, const char*);
void text_set_font(text_t*, font_t*);
void text_set_size(text_t*, uint32_t);

void text_set_rgb(text_t*, uint8_t, uint8_t, uint8_t);
void text_set_rgba(text_t*, uint8_t, uint8_t, uint8_t, uint8_t);
void text_set_rgb_gl(text_t*, vec3);
void text_set_rgba_gl(text_t*, vec4);
void text_set_rgb_hex(text_t*, uint32_t);
void text_set_rgba_hex(text_t*, uint32_t);

void text_move(text_t*, vec2);
void text_move_xy(text_t*, float, float);
void text_wrap(text_t*, uint32_t);

void text_shadow(text_t*, vec2);
void text_shadow_xy(text_t*, float, float);

void text_redraw(text_t*);
void text_render(text_t*);

/*void text_get_bounds(text_t*, vec2);
uint32_t text_line_count(text_t*);*/

void text_destroy(text_t*);

#endif
