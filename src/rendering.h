#pragma once

#include "math2d.h"
#include "math3d.h"
#include "math4d.h"

#include "shapes.h"
#include "util.h"

#include "GL/glew.h"

struct SDL_Window;

namespace render {

struct Camera
{
    math::Vec3 pos;
    math::Quaternion orientation;
    float near = 0.1f;
    float far = 100.0f;
    float near_width = 1.0f; // width of near plane
    bool is_ortho = false;

    // Sets near_width for a given fov.
    // Only valid for perspective cameras
    void set_fov(float fov);
    float get_fov() const;

    // x and y are in pixels from the top left, width and height are width and height of the camera image
    void pixel_ray(int x, int y, int width, int height, math::Vec3* ray_pos, math::Vec3* ray_dir) const;

    // Returns camera_matrix * view_matrix
    math::Mat4 compute_matrix(float aspect_ratio) const;

    void draw_gui();
};

struct LightSource
{
    GLuint fbo;
    GLuint texture;
    int side;
    float aspect_ratio = 1.0f;
    float intensity = 1.0f;

    Camera camera;

    void draw_gui();
};

struct Mesh
{
    GLuint vbo;
    GLuint vao;
    GLuint num_vertices;
};

struct Material
{
    math::Vec3 diffuse_color = math::Vec3(1.0f, 1.0f, 1.0f);
    math::Vec3 specular_color = math::Vec3(0.0f, 0.0f, 0.0f);
    float shininess = 1.0f;

    bool lit = true;
    bool textured = false;

    uint texture_id;
};

typedef u32 RenderObjectIndex;

struct RenderObject
{
    u32 mesh_id;
    u32 material_id;

    const char* filename = "";

    RenderObjectIndex next_group = INVALID_INDEX;
};

extern Array<RenderObject> render_objects;
extern Array<Mesh> meshes;
extern Array<Material> materials;

extern RenderObjectIndex cube;
extern u32 default_material;
extern u32 cube_mesh;

void init_rendering(SDL_Window* window);

LightSource make_light_source(int side);

void prepare_final_draw(Camera camera, LightSource light);
void prepare_lightmap_draw(LightSource light);
void prepare_debug_draw(Camera camera);

void draw_box(Transform3d box);
void draw_object(Transform3d transform, RenderObjectIndex obj_index);
void draw_skybox(u32 texture_index, math::Vec3 camera_pos);
void debug_draw_rectangle(Transform2d rect, float r, float g, float b);
void debug_draw_poly(const math::Vec2* points, u32 count, float r, float g, float b);

// Returns index of render object
RenderObjectIndex load_obj(const char* filename);

// Returns texture id
uint load_texture(const char* path);

int get_screen_width();
int get_screen_height();

// Returns width / height
float get_aspect_ratio();

void present_screen(SDL_Window* window);

}
