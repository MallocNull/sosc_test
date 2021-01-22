#include "font.h"

// SUBSYSTEM //

const char* _vertex_shader =
    "#version 100 \r\n"

    "attribute vec2 coords;"
    "attribute vec2 texuv;"

    "varying vec2 f_texuv;"

    "uniform mat4 trans;"
    "uniform mat4 ortho;"

    "void main() {"
        "gl_Position = ortho * trans * vec4(coords, 0.0, 1.0);"
        "f_texuv = texuv;"
    "}\r\n";
const char* _fragment_shader =
    "#version 100 \n"
    "precision mediump float;\n"

    "varying vec2 f_texuv;\n"

    "uniform vec4 color;\n"
    "uniform sampler2D font;\n"

    "void main() {\n"
        "vec4 fcol = texture2D(font, f_texuv);\n"
        "if(fcol.rgb == vec3(0.0)) discard;\n"
        "gl_FragColor = color * fcol;\n"
    "}\n";

struct {
    shader_t* shader;
    font_t* default_font;

    enum {
        FONT_TRANS,
        FONT_ORTHO,
        FONT_COLOR,
        FONT_FONT
    };
} _ss;

void _font_window_change(SDL_Window* window) {
    shader_start(_ss.shader);

    mat4 ortho;
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    glm_ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f, ortho);
    glUniformMatrix4fv(
        _ATTR(FONT_ORTHO), 1, GL_FALSE, (float*)ortho
    );
}

void font_init_subsystem(SDL_Window* window) {
    _ss.shader = shader_create("font");
    shader_layout(_ss.shader, 2,
        "coords", "texuv"
    );
    shader_source(_ss.shader, SHADER_SRC, 2,
        _vertex_shader,   GL_VERTEX_SHADER,
        _fragment_shader, GL_FRAGMENT_SHADER
    );
    shader_attribs(_ss.shader, 4,
        "trans", "ortho", "color", "font"
    );

    _font_window_change(window);
}

void font_window_changed(SDL_Window* window) {
    _font_window_change(window);
}

void font_set_default(font_t* font) {
    _ss.default_font = font;
}

void font_deinit_subsystem() {
    shader_destroy(_ss.shader);
}

// FONT FUNCTIONS //

font_t* font_load(const char* bmp_path, const char* data_path, int filter) {
    const int buffer_size = 0x111;
    int buffer_read = 0;
    uint8_t buffer[buffer_size];

    FILE* fp = fopen(data_path, "rb");
    if(fp == NULL)
        return NULL;
    buffer_read = fread(buffer, 1, buffer_size, fp);
    fclose(fp);
    if(buffer_read != buffer_size)
        return NULL;
    if(buffer[0x10] != 0)
        return NULL;

    bmp_t* bmp = bmp_load(bmp_path);
    if(bmp == NULL)
        return NULL;

    font_t* font = malloc(sizeof(font_t));

    font->width = bmp->width;
    font->height = bmp->height;
    font->cell_width  = *(uint32_t*)ltoh(buffer + 0x08, 4);
    font->cell_height = *(uint32_t*)ltoh(buffer + 0x0C, 4);

    for(int i = 0; i < 256; ++i) {
        glyph_t* glyph = font->glyphs + i;
        uint32_t width = buffer[0x11 + i];

        glyph->width = (float)width / (float)font->cell_width;
        int x = (font->cell_width * i) % font->width;
        int y = ((font->cell_width * i) / font->width) * font->cell_height;

        glyph->top_left[0] = (float)x / (float)font->width;
        glyph->top_left[1] = (float)y / (float)font->height;

        glyph->bottom_right[0] = (float)(x + width) / (float)font->width;
        glyph->bottom_right[1] =
            (float)(y + font->cell_height) / (float)font->height;
    }

    uint8_t* pixels = bmp_gl_data(bmp, KOA_RGB);

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        font->width, font->height, 0,
        GL_RGB, GL_UNSIGNED_BYTE,
        pixels
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    bmp_unload(bmp);
    free(pixels);

    return font;
}

void font_bind(font_t* font) {
    glBindTexture(GL_TEXTURE_2D, font->texture);
}

void font_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void font_unload(font_t* font) {
    glDeleteTextures(1, font->texture);
    free(font);
}

// TEXT FUNCTIONS //

text_t* text_create(font_t* font) {
    text_t* text = malloc(sizeof(text_t));

    if(font == NULL)
        text->font = _ss.default_font;
    text->text = NULL;
    glm_vec4_copy((vec4){0.f, 0.f, 0.f, 1.f}, text->color);

    text->length    =
    text->size      =
    text->wrap      =
    text->tri_cnt   = 0;

    text->vert_inv  =
    text->texuv_inv = 1;

    glm_vec2_fill(text->shadow, 0.f);

    glm_mat4_identity(text->trans_mat);
    glGenVertexArrays(1, &text->vao);
    glGenBuffers(2, text->buffers);

    glBindVertexArray(text->vao); {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, text->buffers[0]);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, text->buffers[1]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    } glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return text;
}

void text_set(text_t* text, const char* value) {
    uint32_t value_len = strlen(value);
    if(text->text == NULL || strcmp(value, text->text) != 0) {
        text->texuv_inv = 1;
        text->vert_inv = 1;
    }

    free(text->text);
    text->length = value_len;
    text->text = strdup(value);
}

void text_set_font(text_t* text, font_t* font) {
    text->font = font;

    text->texuv_inv = 1;
    text->vert_inv = 1;
}

void text_set_size(text_t* text, uint32_t size) {
    text->size = size;
    text->vert_inv = 1;
}

void text_set_rgb
    (text_t* text, uint8_t r, uint8_t g, uint8_t b)
{
    text_set_rgba(text, r, g, b, 0xFF);
}

void text_set_rgba
    (text_t* text, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    glm_vec4_copy(
        (vec4){
            (float)r / 255.f,
            (float)g / 255.f,
            (float)b / 255.f,
            (float)a / 255.f
        },
        text->color
    );
}

void text_set_rgb_gl(text_t* text, vec3 color) {
    vec4 color4;
    glm_vec4(color, 1.f, color4);
    text_set_rgba_gl(text, color4);
}

void text_set_rgba_gl(text_t* text, vec4 color) {
    glm_vec4_copy(color, text->color);
}

void text_set_rgb_hex(text_t* text, uint32_t color) {
    color &= 0x00FFFFFF;
    color <<= 8;
    color |= 0xFF;

    text_set_rgba_hex(text, color);
}

void text_set_rgba_hex(text_t* text, uint32_t color) {
    text_set_rgba(text,
        (color >> 24) & 0xFF,
        (color >> 16) & 0xFF,
        (color >> 8)  & 0xFF,
         color        & 0xFF
    );
}

void text_move(text_t* text, vec2 coords) {
    glm_translate_make(
        text->trans_mat,
        (vec3){ coords[0], coords[1], 0.f }
    );
}

void text_move_xy(text_t* text, float x, float y) {
    text_move(text, (vec2){ x, y });
}

void text_wrap(text_t* text, uint32_t wrap) {
    text->wrap = wrap;
    text->vert_inv = 1;
}

void text_shadow(text_t* text, vec2 offset) {
    glm_vec2_copy(offset, text->shadow);
}

void text_shadow_xy(text_t* text, float x, float y) {
    text_shadow(text, (vec2){ x, y });
}

void text_redraw(text_t* text) {
    if(!text->vert_inv && !text->texuv_inv)
        return;

    text->tri_cnt = 2 * text->length;

    float* verts = NULL;
    float* texuvs = NULL;

    if(text->vert_inv)
        verts = malloc(sizeof(float) * 6 * text->tri_cnt);
    if(text->texuv_inv)
        texuvs = malloc(sizeof(float) * 6 * text->tri_cnt);

    uint32_t top_x = 0, top_y = 0;
    for(uint32_t i = 0; i < text->length; ++i) {
        glyph_t* glyph = text->font->glyphs + text->text[i];
        uint32_t width  = (uint32_t)(text->size * glyph->width),
                 height = text->size;

        if(top_x + width > text->wrap && text->wrap != 0) {
            top_x = 0;
            top_y += height;
        }

        // !! TRIANGLES WOUND CCW FOR CULLING !! //

        if(verts != NULL) {
            // TRIANGLE 1 //
            // TOP LEFT
            verts[i * 12]     = top_x;
            verts[i * 12 + 1] = top_y;
            // BOTTOM LEFT
            verts[i * 12 + 2] = top_x;
            verts[i * 12 + 3] = top_y + height;
            // TOP RIGHT
            verts[i * 12 + 4] = top_x + width;
            verts[i * 12 + 5] = top_y;

            // TRIANGLE 2 //
            // BOTTOM RIGHT
            verts[i * 12 + 6] = top_x + width;
            verts[i * 12 + 7] = top_y + height;
            // TOP RIGHT
            verts[i * 12 + 8] = top_x + width;
            verts[i * 12 + 9] = top_y;
            // BOTTOM LEFT
            verts[i * 12 + 10] = top_x;
            verts[i * 12 + 11] = top_y + height;
        }

        if(texuvs != NULL) {
            // TRIANGLE 1 //
            // TOP LEFT
            texuvs[i * 12]     = glyph->top_left[0];
            texuvs[i * 12 + 1] = glyph->top_left[1];
            // BOTTOM LEFT
            texuvs[i * 12 + 2] = glyph->top_left[0];
            texuvs[i * 12 + 3] = glyph->bottom_right[1];
            // TOP RIGHT
            texuvs[i * 12 + 4] = glyph->bottom_right[0];
            texuvs[i * 12 + 5] = glyph->top_left[1];

            // TRIANGLE 2 //
            // BOTTOM RIGHT
            texuvs[i * 12 + 6]  = glyph->bottom_right[0];
            texuvs[i * 12 + 7]  = glyph->bottom_right[1];
            // TOP RIGHT
            texuvs[i * 12 + 8]  = glyph->bottom_right[0];
            texuvs[i * 12 + 9]  = glyph->top_left[1];
            // BOTTOM LEFT
            texuvs[i * 12 + 10] = glyph->top_left[0];
            texuvs[i * 12 + 11] = glyph->bottom_right[1];
        }

        top_x += width;
    }

    if(verts != NULL) {
        glBindBuffer(GL_ARRAY_BUFFER, text->buffers[0]);
        glBufferData(
            GL_ARRAY_BUFFER,
            text->tri_cnt * 6 * sizeof(float),
            verts,
            GL_DYNAMIC_DRAW
        );
    }

    if(texuvs != NULL) {
        glBindBuffer(GL_ARRAY_BUFFER, text->buffers[1]);
        glBufferData(
            GL_ARRAY_BUFFER,
            text->tri_cnt * 6 * sizeof(float),
            texuvs,
            GL_DYNAMIC_DRAW
        );
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(verts);
    free(texuvs);

    text->vert_inv = 0;
    text->texuv_inv = 0;
}

void text_render(text_t* text) {
    const vec4 shadow = { 0.f, 0.f, 0.f, 1.f };
    mat4 trans_shadow;

    text_redraw(text);
    shader_start(_ss.shader);

    glActiveTexture(GL_TEXTURE0);
    font_bind(text->font);
    glBindVertexArray(text->vao);

    if(text->shadow[0] != 0 || text->shadow[1] != 0) {
        glm_translate_make(trans_shadow,
                           (vec3){ text->shadow[0], text->shadow[1], -0.1f });
        glm_mat4_mul(trans_shadow, text->trans_mat, trans_shadow);

        glUniformMatrix4fv(
            _ATTR(FONT_TRANS),
            1, GL_FALSE,
            (float*)trans_shadow
        );
        glUniform4fv(
            _ATTR(FONT_COLOR), 1,
            (float*)shadow
        );

        glDrawArrays(GL_TRIANGLES, 0, text->tri_cnt * 3);
    }

    glUniformMatrix4fv(
        _ATTR(FONT_TRANS),
        1, GL_FALSE,
        (float*)text->trans_mat
    );
    glUniform4fv(
        _ATTR(FONT_COLOR), 1,
        (float*)text->color
    );

    glDrawArrays(GL_TRIANGLES, 0, text->tri_cnt * 3);
}

void text_destroy(text_t* text) {
    glDeleteBuffers(2, text->buffers);
    glDeleteVertexArrays(1, &text->vao);

    free(text->text);
    free(text);
}