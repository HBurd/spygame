#include "rendering.h"
#include "util.h"

#include "SDL2/SDL.h"

using math::Vec2;
using math::Mat2;

void Renderer::init(SDL_Renderer* renderer_)
{
    renderer = renderer_;
    SDL_GetRendererOutputSize(renderer, &width, &height);
}

void Renderer::clear() const
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

void Renderer::present()
{
    SDL_RenderPresent(renderer);

    SDL_GetRendererOutputSize(renderer, &width, &height);
}

void Renderer::debug_draw_polygon(Array<Vec2> points) const
{
    // Last point is initiall the final point, then the previous point
    // after the first iteration.

    Vec2 last_point = *(points.end() - 1);
    for (Vec2 point : points)
    {
        int x1;
        int y1;
        transform_to_screen_coords(last_point, &x1, &y1);

        int x2;
        int y2;
        transform_to_screen_coords(point, &x2, &y2);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

        last_point = point;
    }
}

void Renderer::debug_draw_rectangle(Rectangle rect) const
{
    float cosine = cosf(rect.rotation);
    float sine = sinf(rect.rotation);

    Mat2 rotation(cosine, -sine, sine, cosine);

    Vec2 rectangle_points[4] = {
        0.5f * rotation * rect.scale + rect.pos,
        0.5f * rotation * Vec2(-rect.scale.x, rect.scale.y) + rect.pos,
        -0.5f * rotation * rect.scale + rect.pos,
        0.5f * rotation * Vec2(rect.scale.x, -rect.scale.y) + rect.pos,
    };

    debug_draw_polygon(&rectangle_points);
}

void Renderer::transform_to_screen_coords(const Vec2& point, int* x, int* y) const
{
    Vec2 screen_point = point;

    // Switch upward-pointing y-axis to downward-pointing y-axis
    screen_point.y = -screen_point.y;

    // Scale vector to be in pixels
    screen_point *= (float) unit_pixels;

    // Translate vector so that the centre of the screen is the origin
    screen_point += Vec2(width * 0.5f, height * 0.5f);

    /* Note: Truncation is correct here (and rounding is incorrect), when
     * taking into account the offset to the centre of each pixel. In pixel
     * space, the top-left pixel is centred at (0.5, 0.5). */
    *x = (s32) screen_point.x;
    *y = (s32) screen_point.y;
}
