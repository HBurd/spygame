#include "keyboard.h"
#include "SDL2/SDL.h"

bool key_is_held(SDL_Scancode scancode)
{
    int numkeys;
    const Uint8* kb_state = SDL_GetKeyboardState(&numkeys);

    if (scancode >= numkeys)
    {
        return false;
    }

    return kb_state[scancode] == 1;
}
