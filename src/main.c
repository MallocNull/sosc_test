#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>

#include "koa/file.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

struct {
    SDL_Window* window;
    SDL_GLContext ctx;
    int mode, running;
} _g;

int init();
void deinit();

void run() {
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

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(_g.window);
}

int main(int argc, char* argv[]) {
    if(init() < 0)
        return -1;

    _g.running = 1;
    while(_g.running)
        run();

    deinit();
    return 0;
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
    _g.mode = 0;
    return 0;
}

void deinit() {
    SDL_DestroyWindow(_g.window);
    SDL_Quit();
}