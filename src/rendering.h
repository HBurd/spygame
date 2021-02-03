#pragma once

#include "math2d.h"
#include "math3d.h"
#include "math4d.h"

#include "shapes.h"
#include "util.h"

#include "GL/glew.h"

struct SDL_Window;

struct CameraView
{
    float fov = 1.5f;
    float near = 0.1f;
    float far = 100.0f;
    float distance = 5.0f;
    float yaw = 0.0f;
    float pitch = -0.5f;

    math::Vec3 target;
};

struct Renderer
{
    int width = 0;
    int height = 0;

    math::Mat4 camera_matrix;

    GLuint rect_vbo;
    GLuint rect_vao;
    GLuint debug_shader;

    Renderer(SDL_Window* window);

    // This is called internally
    void update_screen_size(SDL_Window* window);

    void clear() const;
    void prepare(CameraView camera);
    void present(SDL_Window* window);

    void debug_draw_rectangle(Rectangle rect, float r, float g, float b) const;

    /* Coordinate systems:
       - Pixel coordinates are measured in pixels from the top-left of the window
       - World coordinates are measured in metres from the world origin
       - Screen coordinates are measured in metres from the screen centre
    */

    /*
    math::Vec2 pixels_to_world(int x, int y) const;
    math::Vec2 pixels_to_screen(int x, int y) const;
    math::Vec2 screen_to_world(math::Vec2 screen) const;
    math::Vec2 world_to_screen(math::Vec2 world) const;
    */
};
