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
    bool is_directional;
    float intensity = 1.0f;

    math::Mat4 matrix;

    math::Vec3 pos;
    math::Quaternion orientation;
};

struct RenderObject
{
    GLuint vbo;
    GLuint vao;
    GLuint num_vertices;

    float diffuse_ratio = 1.0f;
    float shininess = 1.0f;

    bool lit = true;
    bool textured = false;

    uint texture_id;
};

void init_rendering(SDL_Window* window);

LightSource make_light_source(int side, math::Mat4 matrix, bool is_directional);

void prepare_final_draw(math::Mat4 camera_matrix, math::Vec3 camera_dir, LightSource light);
void prepare_lightmap_draw(LightSource light);
void prepare_debug_draw(math::Mat4 camera_matrix);

void draw_box(Transform3d box);
void draw_object(Transform3d transform, RenderObject obj);
void debug_draw_rectangle(Transform2d rect, float r, float g, float b);
void draw_skybox(RenderObject skybox, math::Vec3 camera_pos);

RenderObject load_mesh(const char* filename);
RenderObject create_skybox(const char* filename);

int get_screen_width();
int get_screen_height();

// Returns width / height
float get_aspect_ratio();

void present_screen(SDL_Window* window);

}
