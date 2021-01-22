#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <cglm/cglm.h>
#include <GL/glu.h>

#include <stdio.h>
#include <assert.h>

#include "koa/file.h"
#include "okuu/mesh.h"
#include "okuu/shader.h"
#include "okuu/terrain.h"
#include "okuu/font.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

struct {
    SDL_Window* window;
    SDL_GLContext ctx;
    const uint8_t* keys;
    int mode, running;
    int mouse[3];

    font_t* scape;
    text_t* text;
    mesh_t* monkey;
    terrain_t* map;
} _g;

struct {
    shader_t* shader;

    enum {
        DEF_MODEL,
        DEF_VIEW,
        DEF_PROJ
    };
} _s_def;

int init();
void deinit();
void run();

int main(int argc, char* argv[]) {
    if(init() < 0)
        return -1;

    font_init_subsystem(_g.window);

    _g.scape = font_load(
        "data/fonts/scape2.bmp",
        "data/fonts/scape2.dat",
        GL_NEAREST
    );
    font_set_default(_g.scape);

    _g.text = text_create(NULL);
    //text_set(_g.text, "Welcome to Rune2006");
    text_set_size(_g.text, 16);
    text_set_rgb_hex(_g.text, 0xffff00);

    _g.monkey = mesh_load("data/models/player.rbm");

    color_t path_colors[] = {
        {0x60, 0x60, 0x60, 0xFF},
        {0x9C, 0x00, 0x0E, 0xFF}
    };
    _g.map = terrain_load(
        "data/terrains/map-heights.bmp",
        "data/terrains/map-colors.bmp",
        path_colors, sizeof(path_colors) / sizeof(color_t),
        10, 10
    );

    _s_def.shader = shader_create("default");
    shader_layout(_s_def.shader, 4,
        "vertex", "texuv", "normal", "color"
    );
    shader_source(_s_def.shader, SHADER_FILE, 2,
        "data/shaders/terrain.vert", GL_VERTEX_SHADER,
        "data/shaders/terrain.frag", GL_FRAGMENT_SHADER
    );
    shader_attribs(_s_def.shader, 3,
        "model", "view", "projection"
    );

    _g.keys = SDL_GetKeyboardState(NULL);
    _g.running = 1;
    while(_g.running)
        run();

    deinit();
    return 0;
}

void run() {
    static mat4 model, map, view, projection, mario;
    static vec3 center, eye, pick;
    static float rot_up = 45, rot_around = 45;
    static float x = 0, y = 0;
    static char debug[256] = "";

    static int init = 1;
    if(init) {
        //glm_rotate_make(model, glm_rad(90), (vec3){ 0.f, -1.f, 0.f });
        //glm_translate_make(mario, (vec3){ 4.f, 4.f, 4.f });
        glm_scale_make(mario, (vec3){ 3.f, 3.f, 3.f });

        glm_mat4_identity(model);

        glm_mat4_identity(map);

        glm_mat4_identity(view);

        glm_perspective(
            glm_rad(55),
            (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,
            0.1f, 100.f,
            projection
        );

        shader_start(_s_def.shader); {
            glUniformMatrix4fv(_ATTR(DEF_MODEL), 1, GL_FALSE, (float*)model);
            glUniformMatrix4fv(_ATTR(DEF_VIEW), 1, GL_FALSE, (float*)view);
            glUniformMatrix4fv(_ATTR(DEF_PROJ), 1, GL_FALSE, (float*)projection);
        } shader_stop();

        init = 0;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.f, 0.f, 0.5f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    center[0] = -x;
    center[1] = -(terrain_height(_g.map, x, y) + 2.f);
    center[2] = -y;

    float radius = 4.f + 4.f * (rot_up / 90.f);
    eye[0] = cos(glm_rad(rot_up)) * radius * cos(glm_rad(rot_around));
    eye[1] = radius * sin(glm_rad(rot_up));
    eye[2] = cos(glm_rad(rot_up)) * radius * sin(glm_rad(rot_around));

    glm_lookat(
        eye,
        (vec3){0.f, 0.f, 0.f},
        (vec3){0.f, 1.f, 0.f},
        view
    );

    glm_translate(view, center);
    glm_vec3_inv(center);

    text_set_rgb_hex(_g.text, 0xffff00);
    text_shadow_xy(_g.text, 1, 1);

    shader_start(_s_def.shader); {
        glUniformMatrix4fv(_ATTR(DEF_VIEW), 1, GL_FALSE, (float*)view);

        glUniformMatrix4fv(_ATTR(DEF_MODEL), 1, GL_FALSE, (float*)map);

        glBindVertexArray(_g.map->vao);
        glDrawArrays(GL_TRIANGLES, 0, _g.map->tri_cnt * 3);

        glUniformMatrix4fv(_ATTR(DEF_MODEL), 1, GL_FALSE, (float*)mario);
        mesh_bind(_g.monkey);
        mesh_render(_g.monkey);

        glBindVertexArray(0);
    } shader_stop();

    SDL_PumpEvents();

    if(_g.keys[SDL_SCANCODE_ESCAPE])
        _g.running = 0;
    if(_g.keys[SDL_SCANCODE_F]) {
        _g.mode = !_g.mode;
        SDL_SetWindowFullscreen(
            _g.window,
            _g.mode == 0
            ? 0
            : SDL_WINDOW_FULLSCREEN
        );
    }

    if(_g.keys[SDL_SCANCODE_UP])
        rot_up = glm_min(89.f, rot_up + 1.5f);
    else if(_g.keys[SDL_SCANCODE_DOWN])
        rot_up = glm_max(0.f, rot_up - 1.5f);

    if(_g.keys[SDL_SCANCODE_RIGHT])
        rot_around -= 2.f;
    else if(_g.keys[SDL_SCANCODE_LEFT])
        rot_around += 2.f;

    if(_g.keys[SDL_SCANCODE_W])
        x += .25;
    else if(_g.keys[SDL_SCANCODE_S])
        x -= .25;

    if(_g.keys[SDL_SCANCODE_D])
        y += .25;
    else if(_g.keys[SDL_SCANCODE_A])
        y -= .25;

    //if(_g.keys[SDL_SCANCODE_SPACE]) {
        terrain_move(_g.map, x, y);
        glm_translate_make(map, (vec3){_g.map->top_x, 0, _g.map->top_y});
    //}

    _g.mouse[2] = SDL_GetMouseState(&_g.mouse[0], &_g.mouse[1]);
    float mx = (float)_g.mouse[0] / (float)WINDOW_WIDTH;
    float my = (float)_g.mouse[1] / (float)WINDOW_HEIGHT;
    vec4 mouse = {mx * 2 - 1, (1 - my) * 2 - 1, -1.f, 1.f};
    mat4 imat;

    sprintf(debug, "NDC (%f, %f, %f, %f)", mouse[0], mouse[1], mouse[2], mouse[3]);
    text_set(_g.text, debug);
    text_move_xy(_g.text, 4, 4);
    text_render(_g.text);

    glm_mat4_inv(projection, imat);
    glm_mat4_mulv(imat, mouse, mouse);
    mouse[2] = -1.f;
    mouse[3] = 0.f;

    sprintf(debug, "EYE (%f, %f, %f, %f)", mouse[0], mouse[1], mouse[2], mouse[3]);
    text_set(_g.text, debug);
    text_move_xy(_g.text, 4, 24);
    text_render(_g.text);

    glm_mat4_inv(view, imat);
    glm_mat4_mulv(imat, mouse, mouse);
    glm_vec4_normalize(mouse);

    sprintf(debug, "WORLD (%f, %f, %f, %f)", mouse[0], mouse[1], mouse[2], mouse[3]);
    text_set(_g.text, debug);
    text_move_xy(_g.text, 4, 44);
    text_render(_g.text);

    vec3 pcam;
    glm_vec3_copy(center, pcam);

    sprintf(debug, "PL1LOC (%f, %f, %f)", pcam[0], pcam[1], pcam[2]);
    text_set(_g.text, debug);
    text_move_xy(_g.text, 4, 64);
    text_render(_g.text);

    glm_vec3_add(eye, pcam, pcam);

    sprintf(debug, "CAMLOC (%f, %f, %f)", pcam[0], pcam[1], pcam[2]);
    text_set(_g.text, debug);
    text_move_xy(_g.text, 4, 84);
    text_render(_g.text);

    if(_g.mouse[2] & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if(terrain_pick(_g.map, (vec3){ mouse[0], mouse[1], mouse[2] }, pcam, pick)) {
            //glm_scale_make(mario, (vec3){ 3.f, 3.f, 3.f });
            pick[0] = (int)pick[0] + 0.5f;
            pick[1] += 1.f;
            pick[2] = (int)pick[2] + 0.5f;
            glm_translate_make(mario, pick);
            glm_scale(mario, (vec3){2.f, 2.f, 2.f});
        }
    }

    SDL_Event ev;
    while(SDL_PollEvent(&ev)) {
        if(ev.type == SDL_QUIT)
            _g.running = 0;
        else if(ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            glViewport(0, 0, ev.window.data1, ev.window.data2);
            font_window_changed(_g.window);
        }
        /*else if(ev.type == SDL_MOUSEBUTTONDOWN) {
            float mx = (float)ev.button.x / (float)WINDOW_WIDTH;
            float my = (float)ev.button.y / (float)WINDOW_HEIGHT;
            vec4 mouse = {mx * 2 - 1, (1 - my) * 2 - 1, -1.f, 1.f};
            mat4 imat;

            glm_mat4_inv(projection, imat);
            glm_mat4_mulv(imat, mouse, mouse);
            mouse[2] = -1.f;
            mouse[3] = 0.f;

            glm_mat4_inv(view, imat);
            glm_mat4_mulv(imat, mouse, mouse);



        }*/
    }

    SDL_GL_SwapWindow(_g.window);
}

int init() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    _g.window = SDL_CreateWindow("OpenGL Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    if(_g.window == NULL)
        return -2;

    _g.ctx = SDL_GL_CreateContext(_g.window);
    if(_g.ctx == NULL)
        return -3;

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
        return -4;

    SDL_GL_SetSwapInterval(1);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    _g.mode = 0;
    return 0;
}

void deinit() {
    SDL_DestroyWindow(_g.window);
    SDL_Quit();
}