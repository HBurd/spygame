#pragma once

#include "SDL2/SDL_scancode.h"

#include "util.h"

union SDL_Event;

struct ButtonState
{
    bool down = false;
    bool held = false;

    void handle_down();
    void handle_up();
};

struct MouseState
{
    s32 x = 0;
    s32 y = 0;
    s32 xrel = 0;
    s32 yrel = 0;

    s32 wheel = 0;

    ButtonState left;
    ButtonState middle;
    ButtonState right;
};

void clear_input_events();
ButtonState get_key_state(SDL_Scancode scancode);
void handle_input_event(const SDL_Event* event);

MouseState get_mouse_state();
