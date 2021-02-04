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
    float fov = 0.5f * 3.14159265f;
    float near = 0.1f;
    float far = 100.0f;
    float distance = 5.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;

    math::Vec3 target;

    // Pixel coordinates are measured in pixels from the top-left of the window
    math::Vec3 pixel_direction(int x, int y, int width, int height) const;
    math::Vec3 compute_position() const;
    math::Mat3 compute_rotation() const;
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
};
