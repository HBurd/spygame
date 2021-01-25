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

    uint unit_pixels = 64;

    math::Vec2 camera;


    GLuint rect_vbo;
    GLuint rect_vao;
    GLuint debug_shader;

    Renderer(int width_, int height_);

    void clear() const;
    void present(SDL_Window* window);

    void debug_draw_rectangle(Rectangle rect) const;

    void transform_to_screen_coords(math::Vec2 point, math::Vec2* screen_point) const;
};
