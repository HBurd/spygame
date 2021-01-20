#include "SDL2/SDL.h"
#include "keyboard.h"

#include <cstdio>

typedef unsigned int uint;

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
                case SDL_KEYUP:
                case SDL_KEYDOWN:
                {
                    handke_kb_event(event.key);
                }
            }   
        }
    }

    SDL_Quit();
    return 0;
}
