#include "rendering.h"
#include "util.h"

#include "GL/glew.h"
#include "SDL2/SDL.h"

#include <cstdio> // for shader loading

using math::Vec2;
using math::Mat2;

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

Renderer::Renderer(int width_, int height_)
    : width(width_), height(height_)
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
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    debug_shader = link_program(
        compile_shader("debug_vs.glsl", GL_VERTEX_SHADER),
        compile_shader("debug_fs.glsl", GL_FRAGMENT_SHADER));
}

void Renderer::clear() const
{
    // TODO: Should this be set somewhere else?
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::present(SDL_Window* window)
{
    SDL_GL_SwapWindow(window);

    // Update viewport dimensions if necessary
    int new_width, new_height;
    SDL_GetWindowSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height)
    {
        width = new_width;
        height = new_height;
        glViewport(0, 0, width, height);
    }
}

void Renderer::debug_draw_rectangle(Rectangle rect) const
{
    Vec2 dimensions(width - 1, height - 1);
    dimensions /= unit_pixels;  // put screen dimensions in the right units

    float cosine = cosf(rect.rotation);
    float sine = sinf(rect.rotation);
    Mat2 rotation(cosine, -sine,
                  sine,    cosine);

    glBindVertexArray(rect_vao);
    glUseProgram(debug_shader);

    GLint dimensions_loc = glGetUniformLocation(debug_shader, "dimensions");
    GLint position_loc = glGetUniformLocation(debug_shader, "position");
    GLint rotation_loc = glGetUniformLocation(debug_shader, "rotation");
    GLint scale_loc    = glGetUniformLocation(debug_shader, "scale");

    glUniform2f(dimensions_loc, dimensions.x, dimensions.y);
    glUniform2f(position_loc, rect.pos.x, rect.pos.y);
    glUniformMatrix2fv(rotation_loc, 1, GL_TRUE, rotation.data);
    glUniform2f(scale_loc, rect.scale.x, rect.scale.y);

    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void Renderer::transform_to_screen_coords(Vec2 point, Vec2* screen_point) const
{
    Vec2 result = point;

    // Scale vector to be in pixels
    result *= unit_pixels;
    result.x /= 2.0f * (width - 1);
    result.y /= 2.0f * (height - 1);

    *screen_point = result;
}
