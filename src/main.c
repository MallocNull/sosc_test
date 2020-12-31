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

    vec2 tlocs[10];
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

void reset_text_locs() {
    for(int i = 0; i < 10; ++i) {
        _g.tlocs[i][0] = rand() % (WINDOW_WIDTH / 2);
        _g.tlocs[i][1] = rand() % WINDOW_HEIGHT;
    }
}

int main(int argc, char* argv[]) {
    if(init() < 0)
        return -1;

    font_init_subsystem(_g.window);

    _g.scape = font_load(
        "data/fonts/scape.bmp",
        "data/fonts/scape.dat",
        GL_NEAREST
    );
    font_set_default(_g.scape);

    _g.text = text_create(NULL);
    text_set(_g.text, "flashwave is cool !!!");
    text_set_size(_g.text, 32);
    text_set_rgb_hex(_g.text, 0x55007e);
    reset_text_locs();

    _g.monkey = mesh_load("data/models/player.rbm");

    _g.map = terrain_load(
        "data/terrains/map-heights.bmp",
        "data/terrains/map-colors.bmp",
        10, 10
    );

    _s_def.shader = shader_create("default");
    shader_layout(_s_def.shader, 4,
        "vertex", "texuv", "normal", "color"
    );
    shader_source(_s_def.shader, SHADER_FILE, 2,
        "data/shaders/test.vert", GL_VERTEX_SHADER,
        "data/shaders/test.frag", GL_FRAGMENT_SHADER
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
    static mat4 model, map, view, projection;
    static float rot_up = 45, rot_around = 45;
    static float x = 0, y = 0;

    static int init = 1;
    if(init) {
        //glm_rotate_make(model, glm_rad(90), (vec3){ 0.f, -1.f, 0.f });
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

    float radius = 4.f + 4.f * (rot_up / 90.f);
    glm_lookat(
        (vec3){
            cos(glm_rad(rot_up)) * radius * cos(glm_rad(rot_around)),
            radius * sin(glm_rad(rot_up)),
            cos(glm_rad(rot_up)) * radius * sin(glm_rad(rot_around))
        },
        (vec3){0.f, 0.f, 0.f},
        (vec3){0.f, 1.f, 0.f},
        view
    );

    for(int i = 0; i < 10; ++i) {
        text_move(_g.text, _g.tlocs[i]);
        text_render(_g.text);
    }

    shader_start(_s_def.shader); {
        glUniformMatrix4fv(_ATTR(DEF_VIEW), 1, GL_FALSE, (float*)view);

        glm_translate_make(model, (vec3){-x, -(_g.map->heights[(int)y][(int)x] + 2.f), -y});
        glm_mat4_mul(model, map, model);
        glUniformMatrix4fv(_ATTR(DEF_MODEL), 1, GL_FALSE, (float*)model);

        glBindVertexArray(_g.map->vao);
        glDrawArrays(GL_TRIANGLES, 0, _g.map->tri_cnt * 3);
        glBindVertexArray(0);
    } shader_stop();

    SDL_GL_SwapWindow(_g.window);

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

    if(_g.keys[SDL_SCANCODE_SPACE]) {
        terrain_move(_g.map, x, y);
        glm_translate_make(map, (vec3){__MAX(0, x - CHUNK_SIZE / 2), 0, __MAX(0, y - CHUNK_SIZE / 2)});
    }

    if(_g.keys[SDL_SCANCODE_P])
        reset_text_locs();

    _g.mouse[2] = SDL_GetMouseState(&_g.mouse[0], &_g.mouse[1]);
    if(_g.mouse[2] & SDL_BUTTON(SDL_BUTTON_LEFT)) {

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