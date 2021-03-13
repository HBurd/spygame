#include "rendering.h"
#include "util.h"

#include "math3d.h"
#include "math4d.h"

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "stb_image.h"
#include "fast_obj.h"

#include <cstdio> // for shader loading
#include <cstring> // for memcpy
#include "imgui.h"

#define MAX_TEXTURES 1024

using math::Vec2;
using math::Vec3;
using math::Mat2;
using math::Mat3;
using math::Mat4;

struct Texture
{
    // Should be free'd
    const char* path;
    GLuint id;
};

MAKE_ARRAY(textures, Texture, MAX_TEXTURES);

// Shaders
GLuint light_map_shader;
GLuint simple_shader;
GLuint debug_shader;

// The shader to be used in draw_* functions (except debug)
GLuint selected_shader;

// Basic meshes
render::RenderObjectIndex render::cube;
u32 render::default_material;
u32 render::cube_mesh;
GLuint rect_vbo;
GLuint rect_vao;
GLuint line_vbo;
GLuint line_vao;

int screen_width;
int screen_height;

static GLint compile_shader(const char* filename, GLuint shader_type)
{
    FILE* shader_file = fopen(filename, "r");
    if (!shader_file)
    {
        fprintf(stderr, "Unable to open shader %s.\n", filename);
        return 0;
    }
    fseek(shader_file, 0, SEEK_END);
    size_t file_len = ftell(shader_file);
    GLchar* shader_text = (GLchar*) malloc(file_len + 1); // +1 for null terminator
    rewind(shader_file);
    fread(shader_text, 1, file_len, shader_file);

    // add null terminator
    shader_text[file_len] = 0;

    GLuint shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, &shader_text, NULL);
    glCompileShader(shader);

    free(shader_text);

    GLint log_size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    if (log_size > 1)
    {
        GLchar* info_log = (GLchar*) malloc(log_size);
        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        printf("%s info log: %s\n", filename, info_log);
        free(info_log);
    }

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        fprintf(stderr, "Shader compilation failed.\n");
    }

    return shader;
}

static GLuint link_program(GLint vshader, GLint fshader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        fprintf(stderr, "Shader program linking failed.\n");
    }
    return program;
}

static GLuint load_shader(const char* vname, const char* fname)
{
    return link_program(compile_shader(vname, GL_VERTEX_SHADER),
                        compile_shader(fname, GL_FRAGMENT_SHADER));
}

static void sample_screen_size(SDL_Window* window)
{
    SDL_GetWindowSize(window, &screen_width, &screen_height);
}

struct Vertex
{
    Vec3 position;
    Vec3 normal;
    Vec2 uv;

    Vertex() = default;
    Vertex(Vec3 position_, Vec3 normal_, Vec2 uv_)
        : position(position_), normal(normal_), uv(uv_)
    {}
};

static_assert(sizeof(Vertex) == 2 * sizeof(Vec3) + sizeof(Vec2), "Vertex type must be packed");

// Returns cube positions interleaved with normals and uv coords
static const Array<Vertex> generate_cube_mesh()
{
    static Vertex cube_mesh[36];

    const Vertex front_face[6] = {
        // Lower left
        Vertex(Vec3(-0.5f,  0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.25f, 2.0f / 3.0f)),
        Vertex(Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.25f, 1.0f / 3.0f)),
        Vertex(Vec3( 0.5f, -0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.5f,  1.0f / 3.0f)),

        // Upper right
        Vertex(Vec3( 0.5f, -0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.5f,  1.0f / 3.0f)),
        Vertex(Vec3( 0.5f,  0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.5f,  2.0f / 3.0f)),
        Vertex(Vec3(-0.5f,  0.5f, 0.5f), Vec3(0.0f, 0.0f, 1.0f), Vec2(0.25f, 2.0f / 3.0f)),
    };

    const Mat3 x_rotate = Mat3::RotateX(0.5f * M_PI);
    const Mat3 y_rotate = Mat3::RotateY(0.5f * M_PI);

    for (uint i = 0; i < ARRAY_LENGTH(front_face); ++i)
    {
        cube_mesh[i] = front_face[i];

        cube_mesh[i +     ARRAY_LENGTH(front_face)] = Vertex(y_rotate * front_face[i].position,
                                                             y_rotate * front_face[i].normal,
                                                             Vec2(0.25f, 0.0f) + front_face[i].uv);

        cube_mesh[i + 2 * ARRAY_LENGTH(front_face)] = Vertex(y_rotate * y_rotate * front_face[i].position,
                                                             y_rotate * y_rotate * front_face[i].normal,
                                                             Vec2(0.5f, 0.0f) + front_face[i].uv);

        cube_mesh[i + 3 * ARRAY_LENGTH(front_face)] = Vertex(y_rotate.transpose() * front_face[i].position,
                                                             y_rotate.transpose() * front_face[i].normal,
                                                             Vec2(-0.25f, 0.0f) + front_face[i].uv);

        cube_mesh[i + 4 * ARRAY_LENGTH(front_face)] = Vertex(x_rotate * front_face[i].position,
                                                             x_rotate * front_face[i].normal,
                                                             Vec2(0.0f, -1.0f / 3.0f) + front_face[i].uv);

        cube_mesh[i + 5 * ARRAY_LENGTH(front_face)] = Vertex(x_rotate.transpose() * front_face[i].position,
                                                             x_rotate.transpose() * front_face[i].normal,
                                                             Vec2(0.0f, 1.0f / 3.0f) + front_face[i].uv);
    }

    return Array<Vertex>(&cube_mesh);
}

static u32 create_mesh(Array<Vertex> vertices)
{
    render::Mesh mesh;

    mesh.num_vertices = vertices.size;

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size * sizeof(Vertex), vertices.data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)sizeof(Vec3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(2 * sizeof(Vec3)));

    u32 index = render::meshes.size;

    render::meshes.push(mesh);
    return index;
}

static u32 load_material(fastObjMaterial* mat)
{
    render::Material new_material;

    new_material.diffuse_color = Vec3(mat->Kd);
    new_material.specular_color = Vec3(mat->Ks);
    new_material.shininess = mat->Ns;

    if (mat->map_Kd.path)
    {
        new_material.texture_id = render::load_texture(mat->map_Kd.path);
        new_material.textured = true;
    }

    u32 index = render::materials.size;
    render::materials.push(new_material);
    
    return index;
}

// Public API

namespace render {

MAKE_ARRAY(render_objects, RenderObject, 1024);
MAKE_ARRAY(meshes, Mesh, 1024);
MAKE_ARRAY(materials, Material, 1024);

void Camera::set_fov(float fov)
{
    near_width = 2.0f * near * tanf(0.5f * fov);
}

float Camera::get_fov() const
{
    return 2.0f * atanf(0.5f * near_width / near);
}

Mat4 Camera::compute_matrix(float aspect_ratio) const
{
    Mat3 rotation;
    orientation.inverse().to_matrix(rotation.data);

    Mat4 view = Mat4(rotation) * Mat4::Translate(-pos);

    if (is_ortho)
    {
        return Mat4::Orthographic(near, far, near_width, near_width / aspect_ratio) * view;
    }
    else
    {
        return Mat4::Perspective(near, far, get_fov(), aspect_ratio) * view;
    }
}

void Camera::pixel_ray(int x, int y, int width, int height, Vec3* ray_pos, Vec3* ray_dir) const
{
    // The +0.5f shifts the coordinate to the centre of the pixel
    Vec3 near_plane = Vec3(x, y, 0.0f) - 0.5f * Vec3(width, height, 0.0f) + Vec3(0.5f, 0.5f, 0.0f);

    // Flip the y axis to point up
    near_plane.y = -near_plane.y;

    // Scale to the size of the near plane
    near_plane *= near_width / width;

    // Move the point onto the near_plane
    near_plane.z = -near;

    // Apply camera_view rotation
    near_plane = orientation.apply_rotation(near_plane);

    if (is_ortho)
    {
        *ray_dir = orientation.apply_rotation(Vec3(0.0f, 0.0f, -1.0f));
        *ray_pos = pos + near_plane;
    }
    else
    {
        *ray_dir = near_plane;
        *ray_pos = pos;
    }
}

void Camera::draw_gui()
{
    ImGui::InputFloat3("Position", pos.array());

    ImGui::InputFloat("Near", &near);
    ImGui::InputFloat("Far", &far);

    ImGui::Checkbox("Orthographic?", &is_ortho);

    if (is_ortho)
    {
        ImGui::InputFloat("Near plane width", &near_width);
    }
    else
    {
        float fov = get_fov();
        ImGui::SliderFloat("FOV", &fov, 0.0f, 3.0f);
        set_fov(fov);
    }
}

LightSource make_light_source(int side)
{
    LightSource light;

    light.side = side;

    glGenTextures(1, &light.texture);
    glBindTexture(GL_TEXTURE_2D, light.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, side, side);

    glGenFramebuffers(1, &light.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, light.fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light.texture, 0);

    // Tell OpenGL that the framebuffer does not have a color component
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    assert(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return light;
}

void LightSource::draw_gui()
{
    ImGui::InputFloat("Aspect ratio", &aspect_ratio);
    ImGui::InputFloat("Intensity", &intensity);
    camera.draw_gui();

    ImGui::Image((void*)(intptr_t)texture, ImVec2(200.0f, 200.0f));
}

void prepare_lightmap_draw(LightSource light)
{
    glViewport(0, 0, light.side, light.side);
    glBindFramebuffer(GL_FRAMEBUFFER, light.fbo);

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(light_map_shader);
    selected_shader = light_map_shader;

    GLint loc = glGetUniformLocation(selected_shader, "light");
    glUniformMatrix4fv(loc, 1, GL_TRUE, light.camera.compute_matrix(light.aspect_ratio).data);
}

void init_rendering(SDL_Window* window)
{
    sample_screen_size(window);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    // Generate (2d) square vao
    {
        glGenBuffers(1, &rect_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);

        Vec2 rect_vertices[4] = {
            { 0.5f,  0.5f},
            {-0.5f,  0.5f},
            {-0.5f, -0.5f},
            { 0.5f, -0.5f},
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &rect_vao);
        glBindVertexArray(rect_vao);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    }

    // Generate (2d) line vao
    {
        glGenBuffers(1, &line_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

        Vec2 line_vertices[2] = {Vec2(0.0f, 0.0f), Vec2(1.0f, 0.0f)};

        glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &line_vao);
        glBindVertexArray(line_vao);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    }

    // Load default meshes, materials and objects
    {
        cube_mesh = create_mesh(generate_cube_mesh());
        default_material = materials.size;
        materials.push(Material());

        RenderObject cube_object;
        cube_object.mesh_id = cube_mesh;
        cube_object.material_id = default_material;

        cube = render_objects.size;
        render_objects.push(cube_object);
    }

    debug_shader = load_shader("shaders/debug_vs.glsl", "shaders/debug_fs.glsl");
    simple_shader = load_shader("shaders/simple_vs.glsl", "shaders/simple_fs.glsl");
    light_map_shader = load_shader("shaders/shadow_vs.glsl", "shaders/shadow_fs.glsl");
}

void prepare_final_draw(Camera camera, LightSource light)
{
    glViewport(0, 0, screen_width, screen_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(simple_shader);
    selected_shader = simple_shader;

    GLint loc = glGetUniformLocation(selected_shader, "camera");
    glUniformMatrix4fv(loc, 1, GL_TRUE, camera.compute_matrix(get_aspect_ratio()).data);

    loc = glGetUniformLocation(selected_shader, "camera_pos");
    glUniform3fv(loc, 1, camera.pos.array());

    loc = glGetUniformLocation(selected_shader, "light_pos");
    glUniform3fv(loc, 1, light.camera.pos.array());

    loc = glGetUniformLocation(selected_shader, "light");
    glUniformMatrix4fv(loc, 1, GL_TRUE, light.camera.compute_matrix(light.aspect_ratio).data);

    loc = glGetUniformLocation(selected_shader, "light_direction");
    glUniform3fv(loc, 1, light.camera.orientation.apply_rotation(Vec3(0.0f, 0.0f, -1.0f)).array());

    loc = glGetUniformLocation(selected_shader, "intensity");
    glUniform1f(loc, light.intensity);

    loc = glGetUniformLocation(selected_shader, "is_directional");
    glUniform1i(loc, light.camera.is_ortho);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, light.texture);
}

void draw_box(Transform3d box)
{
    draw_object(box, cube);
}

void draw_skybox(u32 texture_id, Vec3 camera_pos)
{
    Mesh mesh = meshes[cube];

    glBindVertexArray(mesh.vao);

    Mat3 rotation;
    Vec3 scale(1.0f, 1.0f, 1.0f);

    GLint loc = glGetUniformLocation(selected_shader, "rotation");
    glUniformMatrix3fv(loc, 1, GL_TRUE, rotation.data);

    loc = glGetUniformLocation(selected_shader, "scale");
    glUniform3fv(loc, 1, scale.array());

    loc = glGetUniformLocation(selected_shader, "position");
    glUniform3fv(loc, 1, camera_pos.array());

    loc = glGetUniformLocation(selected_shader, "lit");
    glUniform1i(loc, false);

    loc = glGetUniformLocation(selected_shader, "textured");
    glUniform1i(loc, true);

    loc = glGetUniformLocation(selected_shader, "color_texture");
    glUniform1i(loc, 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[texture_id].id);

    glDepthMask(GL_FALSE);
    glFrontFace(GL_CW);
    glDrawArrays(GL_TRIANGLES, 0, mesh.num_vertices);
    glFrontFace(GL_CCW);
    glDepthMask(GL_TRUE);
}

void draw_object(Transform3d transform, RenderObjectIndex obj_index)
{
    while (obj_index != INVALID_INDEX)
    {
        RenderObject obj = render_objects[obj_index];

        Mesh mesh = meshes[obj.mesh_id];
        Material mat = materials[obj.material_id];

        Mat3 rotation = Mat3::RotateZ(transform.rotation);

        glBindVertexArray(mesh.vao);

        GLint loc = glGetUniformLocation(selected_shader, "rotation");
        glUniformMatrix3fv(loc, 1, GL_TRUE, rotation.data);

        loc = glGetUniformLocation(selected_shader, "scale");
        glUniform3fv(loc, 1, transform.scale.array());

        loc = glGetUniformLocation(selected_shader, "position");
        glUniform3fv(loc, 1, transform.pos.array());

        loc = glGetUniformLocation(selected_shader, "diffuse_color");
        glUniform3fv(loc, 1, mat.diffuse_color.array());

        loc = glGetUniformLocation(selected_shader, "specular_color");
        glUniform3fv(loc, 1, mat.specular_color.array());

        loc = glGetUniformLocation(selected_shader, "shininess");
        glUniform1f(loc, mat.shininess);

        loc = glGetUniformLocation(selected_shader, "lit");
        glUniform1i(loc, mat.lit);

        loc = glGetUniformLocation(selected_shader, "textured");
        glUniform1i(loc, mat.textured);

        // TODO: This has to be changed to allow multiple (shadowed) light sources
        // Also this only really has to be done once per shader
        loc = glGetUniformLocation(selected_shader, "light_depth");
        glUniform1i(loc, 0);
        loc = glGetUniformLocation(selected_shader, "color_texture");
        glUniform1i(loc, 1);

        if (mat.textured)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textures[mat.texture_id].id);
        }

        glDrawArrays(GL_TRIANGLES, 0, mesh.num_vertices);

        obj_index = obj.next_group;
    }
}

RenderObjectIndex load_obj(const char* filename)
{
    RenderObjectIndex last_render_object = INVALID_INDEX;

    fastObjMesh* mesh = fast_obj_read(filename);

    const uint face_vertices = 3;

    // TODO: I think all submeshes can be loaded into the same vbo/vao, and rendered
    // separately with different starting indices and counts. Maybe this will be more
    // efficient than using separate VBOs for each, but I'm not sure.
    for (uint g = 0; g < mesh->group_count; ++g)
    {
        uint group_vertices = mesh->groups[g].face_count * face_vertices;

        if (!group_vertices) continue;

        // TODO: I'd rather not do an allocation for each iteration
        Vertex* vertex_data = new Vertex[group_vertices];
        Array<Vertex> vertices(vertex_data, 0, group_vertices);

        uint group_material = mesh->face_materials[mesh->groups[g].face_offset];

        for (uint f = mesh->groups[g].face_offset; f < mesh->groups[g].face_offset + mesh->groups[g].face_count; ++f)
        {
            if (mesh->face_vertices[f] != face_vertices)
            {
                fprintf(stderr, "Error loading mesh. Face has %u vertices, "
                        "but only faces with %u vertices are supported.\n",
                        mesh->face_vertices[f], face_vertices);
                assert(false);
            }

            if (mesh->face_materials[f] != group_material)
            {
                fprintf(stderr, "Error loading mesh. A group must only have a single material, "
                        "but this group has more than one material.");
                assert(false);
            }

            for (uint v = 0; v < face_vertices; ++v)
            {
                uint pos_index =  mesh->indices[face_vertices * f + v].p;
                uint normal_idx = mesh->indices[face_vertices * f + v].n;
                uint uv_idx =     mesh->indices[face_vertices * f + v].t;

                vertices.push(Vertex(Vec3(mesh->positions + 3 * pos_index),
                                     Vec3(mesh->normals   + 3 * normal_idx),
                                     Vec2(mesh->texcoords + 2 * uv_idx)));
            }
        }

        RenderObject render_object;
        render_object.next_group = last_render_object;
        render_object.mesh_id = create_mesh(vertices);
        render_object.material_id = load_material(&mesh->materials[group_material]);

        last_render_object = render_objects.size;
        render_objects.push(render_object);

        delete[] vertex_data;
    }

    render_objects[last_render_object].filename = filename;

    fast_obj_destroy(mesh);

    return last_render_object;
}

void prepare_debug_draw(Camera camera)
{
    glViewport(0, 0, screen_width, screen_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(debug_shader);

    GLint loc = glGetUniformLocation(debug_shader, "camera");
    glUniformMatrix4fv(loc, 1, GL_TRUE, camera.compute_matrix(get_aspect_ratio()).data);
}

void debug_draw_rectangle(Transform2d rect, float r, float g, float b)
{
    Mat2 rotation = Mat2::Rotation(rect.rotation);

    glBindVertexArray(rect_vao);
    glUseProgram(debug_shader);

    GLint loc = glGetUniformLocation(debug_shader, "color");
    glUniform3f(loc, r, g, b);

    loc = glGetUniformLocation(debug_shader, "position");
    glUniform2fv(loc, 1, rect.pos.array());

    loc = glGetUniformLocation(debug_shader, "rotation");
    glUniformMatrix2fv(loc, 1, GL_TRUE, rotation.data);

    loc = glGetUniformLocation(debug_shader, "scale");
    glUniform2fv(loc, 1, rect.scale.array());

    glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glEnable(GL_DEPTH_TEST);
}

void debug_draw_poly(const Vec2* points, u32 count, float r, float g, float b)
{
    for (uint i = 0; i < count; ++i)
    {
        Vec2 edge = points[(i + 1) % count] - points[i];

        // The rotation matrix transforms [1 0] into [edge.x, edge.y].
        // Technically the rotaiton matrix is acting as rotation and scale.
        Mat2 rotation(edge.x, 0.0f,
                      edge.y, 0.0f);

        Vec2 scale(1.0f, 1.0f);

        glBindVertexArray(line_vao);
        glUseProgram(debug_shader);

        GLint loc = glGetUniformLocation(debug_shader, "color");
        glUniform3f(loc, r, g, b);

        loc = glGetUniformLocation(debug_shader, "position");
        glUniform2fv(loc, 1, points[i].array());

        loc = glGetUniformLocation(debug_shader, "rotation");
        glUniformMatrix2fv(loc, 1, GL_TRUE, rotation.data);

        loc = glGetUniformLocation(debug_shader, "scale");
        glUniform2fv(loc, 1, scale.array());

        glDisable(GL_DEPTH_TEST);

        glDrawArrays(GL_LINES, 0, 2);

        glEnable(GL_DEPTH_TEST);
    }
}

void present_screen(SDL_Window* window)
{
    SDL_GL_SwapWindow(window);
    sample_screen_size(window);
}

int get_screen_width()
{
    return screen_width;
}

int get_screen_height()
{
    return screen_height;
}

// Returns width / height
float get_aspect_ratio()
{
    return float(screen_width) / float(screen_height);
}

// This won't load a texture if it has already been loaded.
// Returns the index of the texture in textures
uint load_texture(const char* path)
{
    for (uint i = 0; i < textures.size; ++i)
    {
        if (strcmp(path, textures[i].path) == 0)
        {
            return i;
        }
    }

    size_t path_len = strlen(path) + 1;

    char* new_path = (char*) malloc(path_len);
    memcpy(new_path, path, path_len);

    Texture new_texture;
    new_texture.path = new_path;

    // Generate the texture and load the image into it
    int w, h, n;
    u8* image = stbi_load(path, &w, &h, &n, 3);

    glGenTextures(1, &new_texture.id);
    glBindTexture(GL_TEXTURE_2D, new_texture.id);

    // TODO: can these be read from mtl?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);

    uint texture_id = textures.size;
    textures.push(new_texture);
    return texture_id;
}

}
