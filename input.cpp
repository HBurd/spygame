#include "input.h"
#include "util.h"
#include "SDL2/SDL.h"

ButtonState key_states[SDL_NUM_SCANCODES];
MouseState mouse_state;

void ButtonState::handle_down()
{
    if (!held)
    {
        down = true;
        held = true;
    }
}

void ButtonState::handle_up()
{
    held = false;
}

void clear_input_events()
{
    for (auto& key : key_states)
    {
        key.down = false;
    }

    mouse_state.left.down = false;
    mouse_state.middle.down = false;
    mouse_state.right.down = false;
}

void handle_input_event(const SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN)
    {
        assert(event->key.keysym.scancode < ARRAY_LENGTH(key_states));
        key_states[event->key.keysym.scancode].handle_down();
    }
    else if (event->type == SDL_KEYUP)
    {
        assert(event->key.keysym.scancode < ARRAY_LENGTH(key_states));
        key_states[event->key.keysym.scancode].handle_up();
    }
    else if (event->type == SDL_MOUSEMOTION)
    {
        mouse_state.x = event->motion.x;
        mouse_state.y = event->motion.y;
        mouse_state.xrel = event->motion.xrel;
        mouse_state.yrel = event->motion.yrel;
    }
    else if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            mouse_state.left.handle_down();
        }
        else if (event->button.button == SDL_BUTTON_MIDDLE)
        {
            mouse_state.middle.handle_down();
        }
        else if (event->button.button == SDL_BUTTON_RIGHT)
        {
            mouse_state.right.handle_down();
        }
    }
    else if (event->type == SDL_MOUSEBUTTONUP)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            mouse_state.left.handle_up();
        }
        else if (event->button.button == SDL_BUTTON_MIDDLE)
        {
            mouse_state.middle.handle_up();
        }
        else if (event->button.button == SDL_BUTTON_RIGHT)
        {
            mouse_state.right.handle_up();
        }
    }
}

ButtonState get_key_state(SDL_Scancode scancode)
{
    assert(scancode < ARRAY_LENGTH(key_states));
    return key_states[scancode];
}

MouseState get_mouse_state()
{
    return mouse_state;
}
