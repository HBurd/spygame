#include "rendering.h"
#include "util.h"

#include "math3d.h"
#include "math4d.h"

#include "GL/glew.h"
#include "SDL2/SDL.h"

#include <cstdio> // for shader loading
#include <cstring> // for memcpy

using math::Vec2;
using math::Vec3;
using math::Mat2;
using math::Mat3;
using math::Mat4;

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

Renderer::Renderer(SDL_Window* window)
{
    update_screen_size(window);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

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
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    debug_shader = link_program(
        compile_shader("debug_vs.glsl", GL_VERTEX_SHADER),
        compile_shader("debug_fs.glsl", GL_FRAGMENT_SHADER));
}

void Renderer::update_screen_size(SDL_Window* window)
{
    // Only update if necessary
    int new_width, new_height;
    SDL_GetWindowSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height)
    {
        width = new_width;
        height = new_height;
        glViewport(0, 0, width, height);
    }
}

void Renderer::clear() const
{
    // TODO: Should this be set somewhere else?
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::prepare(CameraView camera)
{
    clear();

    camera_matrix =
        Mat4::Perspective(camera.near, camera.far, camera.fov, float(width)/float(height))
        * Mat4::Translate(Vec3(0.0f, 0.0f, -camera.distance))
        * Mat4(Mat3::RotateX(camera.pitch)
               * Mat3::RotateZ(camera.yaw))
        * Mat4::Translate(-camera.target);
}

void Renderer::present(SDL_Window* window)
{
    SDL_GL_SwapWindow(window);
    update_screen_size(window);
}

void Renderer::debug_draw_rectangle(Rectangle rect, float r, float g, float b) const
{
    float cosine = cosf(rect.rotation);
    float sine = sinf(rect.rotation);
    Mat2 rotation(cosine, -sine,
                  sine,    cosine);

    glBindVertexArray(rect_vao);
    glUseProgram(debug_shader);

    GLint camera_loc   = glGetUniformLocation(debug_shader, "camera");
    GLint color_loc    = glGetUniformLocation(debug_shader, "color");
    GLint position_loc = glGetUniformLocation(debug_shader, "position");
    GLint rotation_loc = glGetUniformLocation(debug_shader, "rotation");
    GLint scale_loc    = glGetUniformLocation(debug_shader, "scale");

    glUniformMatrix4fv(camera_loc, 1, GL_TRUE, camera_matrix.data);
    glUniform3f(color_loc, r, g, b);
    glUniform2f(position_loc, rect.pos.x, rect.pos.y);
    glUniformMatrix2fv(rotation_loc, 1, GL_TRUE, rotation.data);
    glUniform2f(scale_loc, rect.scale.x, rect.scale.y);

    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

/*
Vec2 Renderer::pixels_to_screen(int x, int y) const
{
    // The +0.5f shifts the coordinate to the centre of the pixel
    Vec2 screen_coord = Vec2(x, y) - 0.5f * Vec2(width, height) + Vec2(0.5f, 0.5f);
    screen_coord /= pixels_per_metre;
    screen_coord.y = -screen_coord.y;
    return screen_coord;
}

Vec2 Renderer::pixels_to_world(int x, int y) const
{
    return screen_to_world(pixels_to_screen(x, y));
}

Vec2 Renderer::screen_to_world(Vec2 screen) const
{
    return screen + camera;
}

Vec2 Renderer::world_to_screen(Vec2 world) const
{
    return world - camera;
}
*/
