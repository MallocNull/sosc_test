#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <assert.h>

#include <cglm/cglm.h>

#include "koa/file.h"
#include "okuu/mesh.h"
#include "okuu/shader.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

struct {
    SDL_Window* window;
    SDL_GLContext ctx;
    int mode, running;

    mesh_t* monkey;
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

    _g.monkey = mesh_load("data/monkey.rbm");

    _s_def.shader = shader_create("default");
    shader_source(_s_def.shader, 2,
        "shaders/test.vert", GL_VERTEX_SHADER,
        "shaders/test.frag", GL_FRAGMENT_SHADER
    );
    shader_attribs(_s_def.shader, 3,
        "model", "view", "projection"
    );

    _g.running = 1;
    while(_g.running)
        run();

    deinit();
    return 0;
}

void run() {
    static mat4 model, view, projection;

    static int init = 1;
    if(init) {
        //glm_translate_make(model, (vec3){ 0.0f, 0.f, -3.f });
        //glm_rotate(model, glm_rad(180), (vec3){0.f, 1.f, 0.f});
        glm_rotate_make(model, glm_rad(180), (vec3){ 0.f, 1.f, 0.f });
        //glm_mat4_identity(model);

        //glm_mat4_identity(view);

        glm_lookat(
            (vec3){3.f, 3.f, 3.f},
            (vec3){0.f, 0.f, 0.f},
            (vec3){0.f, 1.f, 0.f},
            view
        );

        glm_perspective(
            glm_rad(45),
            (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,
            0.1f, 10.f,
            projection
        );

        /*glm_perspective_default(
            (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,
            projection
        );*/

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

    shader_start(_s_def.shader); {
        mesh_bind(_g.monkey);
        mesh_render(_g.monkey);
        mesh_unbind();
    } shader_stop();

    SDL_GL_SwapWindow(_g.window);

    SDL_Event ev;
    while(SDL_PollEvent(&ev)) {
        if(ev.type == SDL_KEYDOWN) {
            switch(ev.key.keysym.sym) {
                case SDLK_ESCAPE:
                    _g.running = 0;
                    break;
                case 'f':
                    _g.mode = !_g.mode;
                    SDL_SetWindowFullscreen(
                        _g.window,
                        _g.mode == 0
                        ? 0
                        : SDL_WINDOW_FULLSCREEN
                    );
                    break;
            }
        } else if(ev.type == SDL_QUIT) {
            _g.running = 0;
        }
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