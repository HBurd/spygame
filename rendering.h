#pragma once

#include "math2d.h"
#include "shapes.h"
#include "util.h"

#include "GL/glew.h"

struct SDL_Window;

struct Renderer
{
    int width = 0;
    int height = 0;

    uint pixels_per_metre = 64;

    math::Vec2 camera;

    GLuint rect_vbo;
    GLuint rect_vao;
    GLuint debug_shader;

    Renderer(int width_, int height_);

    void clear() const;
    void present(SDL_Window* window);

    void debug_draw_rectangle(Rectangle rect, float r, float g, float b) const;

    /* Coordinate systems:
       - Pixel coordinates are measured in pixels from the top-left of the window
       - World coordinates are measured in metres from the world origin
       - Screen coordinates are measured in metres from the screen centre
    */

    math::Vec2 pixels_to_world(int x, int y) const;
    math::Vec2 pixels_to_screen(int x, int y) const;
    math::Vec2 screen_to_world(math::Vec2 screen) const;
    math::Vec2 world_to_screen(math::Vec2 world) const;
};
