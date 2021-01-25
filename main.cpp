#include "SDL2/SDL.h"
#include "keyboard.h"
#include "util.h"
#include "game.h"
#include "rendering.h"

#include "GL/glew.h"
#include <cstdio>

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
            "...",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            INITIAL_SCREEN_WIDTH,
            INITIAL_SCREEN_HEIGHT,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow returned null.\nMore info: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GLContext gl_ctxt = SDL_GL_CreateContext(window);
    if (!gl_ctxt)
    {
        fprintf(stderr, "SDL_GL_CreateContext returned null.\nMore info: %s", SDL_GetError());
        return 1;
    }

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        fprintf(stderr, "glewInit did not return GLEW_OK.\nMore info: %s", glewGetErrorString(glew_status));
        return 1;
    }


    init_game();

    Renderer renderer(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT);

    bool running = true;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                {
                   running = false;
                }
            }
        }

        if (key_is_held(SDL_SCANCODE_ESCAPE))
        {
            running = false;
        }

        // TODO: frame timing
        update_game(1.0f / 60.0f);

        render_game(&renderer);
        renderer.present(window);
    }

    SDL_Quit();
    return 0;
}
