#pragma once

#include "math2d.h"
#include "shapes.h"
#include "util.h"

struct SDL_Renderer;

struct Renderer
{
    int width = 0;
    int height = 0;

    uint unit_pixels = 64;

    math::Vec2 camera;

    SDL_Renderer* renderer = nullptr;

    void init(SDL_Renderer* renderer_);

    void clear() const;
    void present();

    void debug_draw_polygon(Array<math::Vec2> points) const;
    void debug_draw_rectangle(Rectangle rect) const;

    void transform_to_screen_coords(const math::Vec2& point, int* x, int* y) const;
};
