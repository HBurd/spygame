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

using math::Vec2;
using math::Vec3;
using math::Mat2;
using math::Mat3;
using math::Mat4;

// Shaders
GLuint light_map_shader;
GLuint simple_shader;
GLuint debug_shader;

// The shader to be used in draw_* functions (except debug)
GLuint selected_shader;

// Basic meshes
render::RenderObject cube;
GLuint rect_vbo;
GLuint rect_vao;

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

static render::RenderObject create_render_object(Array<Vertex> vertices)
{
    render::RenderObject obj;

    obj.num_vertices = vertices.size;

    glGenBuffers(1, &obj.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj.vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size * sizeof(Vertex), vertices.data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &obj.vao);
    glBindVertexArray(obj.vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)sizeof(Vec3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(2 * sizeof(Vec3)));

    return obj;
}

// Public API

namespace render {

void LightSource::init(int side_)
{
    side = side_;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, side, side);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

    // Tell OpenGL that the framebuffer does not have a color component
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    assert(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initialized = true;
}

void LightSource::prepare_draw()
{
    assert(initialized);
    glViewport(0, 0, side, side);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(light_map_shader);
    selected_shader = light_map_shader;

    GLint loc = glGetUniformLocation(selected_shader, "light");
    glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.data);
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

        debug_shader = load_shader("shaders/debug_vs.glsl", "shaders/debug_fs.glsl");
    }

    // Generate cube rener object
    cube = create_render_object(generate_cube_mesh());
    simple_shader = load_shader("shaders/simple_vs.glsl", "shaders/simple_fs.glsl");

    light_map_shader = load_shader("shaders/shadow_vs.glsl", "shaders/shadow_fs.glsl");
}

void prepare_final_draw(Mat4 camera_matrix, LightSource light)
{
    glViewport(0, 0, screen_width, screen_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(simple_shader);
    selected_shader = simple_shader;

    GLint loc = glGetUniformLocation(selected_shader, "camera");
    glUniformMatrix4fv(loc, 1, GL_TRUE, camera_matrix.data);

    loc = glGetUniformLocation(selected_shader, "light_pos");
    glUniform3fv(loc, 1, light.pos.array());

    loc = glGetUniformLocation(selected_shader, "light");
    glUniformMatrix4fv(loc, 1, GL_TRUE, light.matrix.data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, light.texture);
}

void draw_box(Transform3d box)
{
    draw_object(box, cube);
}

void draw_skybox(RenderObject skybox, Vec3 camera_pos)
{
    glBindVertexArray(skybox.vao);

    Mat3 rotation;
    Vec3 scale(1.0f, 1.0f, 1.0f);

    GLint loc = glGetUniformLocation(selected_shader, "rotation");
    glUniformMatrix3fv(loc, 1, GL_TRUE, rotation.data);

    loc = glGetUniformLocation(selected_shader, "scale");
    glUniform3fv(loc, 1, scale.array());

    loc = glGetUniformLocation(selected_shader, "position");
    glUniform3fv(loc, 1, camera_pos.array());

    loc = glGetUniformLocation(selected_shader, "lit");
    glUniform1i(loc, skybox.lit);

    loc = glGetUniformLocation(selected_shader, "textured");
    glUniform1i(loc, skybox.textured);

    loc = glGetUniformLocation(selected_shader, "color_texture");
    glUniform1i(loc, 1);

    if (skybox.textured)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, skybox.texture);
    }

    glDepthMask(GL_FALSE);
    glFrontFace(GL_CW);
    glDrawArrays(GL_TRIANGLES, 0, skybox.num_vertices);
    glFrontFace(GL_CCW);
    glDepthMask(GL_TRUE);
}

void draw_object(Transform3d transform, RenderObject obj)
{
    Mat3 rotation = Mat3::RotateZ(transform.rotation);

    glBindVertexArray(obj.vao);

    GLint loc = glGetUniformLocation(selected_shader, "rotation");
    glUniformMatrix3fv(loc, 1, GL_TRUE, rotation.data);

    loc = glGetUniformLocation(selected_shader, "scale");
    glUniform3fv(loc, 1, transform.scale.array());

    loc = glGetUniformLocation(selected_shader, "position");
    glUniform3fv(loc, 1, transform.pos.array());

    loc = glGetUniformLocation(selected_shader, "lit");
    glUniform1i(loc, obj.lit);

    loc = glGetUniformLocation(selected_shader, "textured");
    glUniform1i(loc, obj.textured);

    // TODO: This has to be changed to allow multiple (shadowed) light sources
    // Also this only really has to be done once per shader
    loc = glGetUniformLocation(selected_shader, "light_depth");
    glUniform1i(loc, 0);
    loc = glGetUniformLocation(selected_shader, "color_texture");
    glUniform1i(loc, 1);

    if (obj.textured)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, obj.texture);
    }

    glDrawArrays(GL_TRIANGLES, 0, obj.num_vertices);
}

void prepare_debug_draw(Mat4 camera_matrix)
{
    glViewport(0, 0, screen_width, screen_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(debug_shader);

    GLint loc = glGetUniformLocation(debug_shader, "camera");
    glUniformMatrix4fv(loc, 1, GL_TRUE, camera_matrix.data);
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

RenderObject load_mesh(const char* filename)
{
    fastObjMesh* mesh = fast_obj_read(filename);

    const uint face_vertices = 3;
    const uint num_vertices = mesh->face_count * face_vertices;

    Vertex* vertex_data = new Vertex[num_vertices];
    Array<Vertex> vertices(vertex_data, 0, num_vertices);

    for (uint f = 0; f < mesh->face_count; ++f)
    {
        if (mesh->face_vertices[f] != face_vertices)
        {
            fprintf(stderr, "Error loading mesh. Face has %u vertices, "
                    "but only faces with %u vertices are supported.\n",
                    mesh->face_vertices[f], face_vertices);
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

    fast_obj_destroy(mesh);

    RenderObject result = create_render_object(vertices);

    delete[] vertex_data;

    return result;
}

RenderObject create_skybox(const char* filename)
{
    // TODO: Something to think about: Can RenderObjects share opengl objects?
    // What do we do when destroying a skybox? How do we know not to destroy cube?
    RenderObject skybox = cube;
    skybox.textured = true;
    skybox.lit = false;

    int w, h, n;
    u8* image = stbi_load(filename, &w, &h, &n, 3);

    glGenTextures(1, &skybox.texture);
    glBindTexture(GL_TEXTURE_2D, skybox.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);

    return skybox;
}

}
