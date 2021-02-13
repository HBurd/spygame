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


void init_rendering(SDL_Window* window);
void prepare_final_draw(math::Mat4 camera_matrix, LightSource light);
void draw_box(Transform3d box);
int get_screen_width();
int get_screen_height();

// Returns width / height
float get_aspect_ratio();

void present_screen(SDL_Window* window);

}
