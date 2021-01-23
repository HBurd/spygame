#include "SDL2/SDL.h"
#include "keyboard.h"
#include "util.h"
#include "game.h"

#include <cstdio>

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

uint screen_width = INITIAL_SCREEN_WIDTH;
uint screen_height = INITIAL_SCREEN_HEIGHT;

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
            "...",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            screen_width,
            screen_height,
            0);

    SDL_Renderer* renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_PRESENTVSYNC);

    init_game(renderer);

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

        render_game();
    }

    SDL_Quit();
    return 0;
}
