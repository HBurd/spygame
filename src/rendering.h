#pragma once

#include "math2d.h"
#include "math3d.h"
#include "math4d.h"

#include "shapes.h"
#include "util.h"

#include "GL/glew.h"

struct SDL_Window;

struct Renderer
{
    int width = 0;
    int height = 0;

    math::Vec3 light_pos;
    math::Mat4 light_matrix;

    math::Mat4 camera_matrix;

    GLuint rect_vbo;
    GLuint rect_vao;
    GLuint debug_shader;

    GLuint cube_vbo;
    GLuint cube_vao;
    GLuint simple_shader;

    GLuint shadow_fbo;
    GLuint shadow_tex;
    GLuint shadow_shader;

    GLuint selected_shader;

    Renderer(SDL_Window* window);

    // This is called automatically by the constructor and after present()
    void update_screen_size(SDL_Window* window);

    void prepare_final_draw();
    void prepare_shadow_draw();
    void present(SDL_Window* window);

    void debug_draw_rectangle(Transform2d rect, float r, float g, float b) const;
    void draw_box(Transform3d box);
};
