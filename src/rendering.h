#pragma once

#include "math2d.h"
#include "math3d.h"
#include "math4d.h"

#include "shapes.h"
#include "util.h"

#include "GL/glew.h"

struct SDL_Window;

namespace render {

struct LightSource
{
    GLuint fbo;
    GLuint texture;
    int side;

    math::Mat4 matrix;
    math::Vec3 pos;

    bool initialized = false;

    // Must be called after init_rendering().
    // RAII isn't used because it wouldn't allow statically allocating light sources.
    void init(int side_);
    void prepare_draw();
};

struct RenderObject
{
    GLuint vbo;
    GLuint vao;
    GLuint texture;
    GLuint num_vertices;

    bool lit = true;
    bool textured = false;
};

void init_rendering(SDL_Window* window);

void prepare_final_draw(math::Mat4 camera_matrix, LightSource light);
void prepare_debug_draw(math::Mat4 camera_matrix);

void draw_box(Transform3d box);
void debug_draw_rectangle(Transform2d rect, float r, float g, float b);
void draw_skybox(RenderObject skybox, math::Vec3 camera_pos);

RenderObject create_skybox(const char* filename);

int get_screen_width();
int get_screen_height();

// Returns width / height
float get_aspect_ratio();

void present_screen(SDL_Window* window);

}
