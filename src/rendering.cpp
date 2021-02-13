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

// Shaders
GLuint light_map_shader;
GLuint simple_shader;
GLuint debug_shader;

// The shader to be used in draw_* functions
GLuint selected_shader;

// OpelGL objects for basic meshes
GLuint rect_vbo;
GLuint rect_vao;

GLuint cube_vbo;
GLuint cube_vao;

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

// Returns cube positions interleaved with normals
static const Vec3* generate_cube_mesh()
{
    static Vec3 cube_mesh[72];

    const Vec3 front_face[12] = {
        // Lower left
        Vec3(-0.5f,  0.5f, 0.5f),
        Vec3( 0.0f,  0.0f, 1.0f),
        Vec3(-0.5f, -0.5f, 0.5f),
        Vec3( 0.0f,  0.0f, 1.0f),
        Vec3( 0.5f, -0.5f, 0.5f),
        Vec3( 0.0f,  0.0f, 1.0f),

        // Upper right
        Vec3( 0.5f, -0.5f, 0.5f),
        Vec3( 0.0f,  0.0f, 1.0f),
        Vec3( 0.5f,  0.5f, 0.5f),
        Vec3( 0.0f,  0.0f, 1.0f),
        Vec3(-0.5f,  0.5f, 0.5f),
        Vec3( 0.0f,  0.0f, 1.0f),
    };

    const Mat3 x_rotate = Mat3::RotateX(0.5f * M_PI);
    const Mat3 y_rotate = Mat3::RotateY(0.5f * M_PI);

    for (int i = 0; i < ARRAY_LENGTH(front_face); ++i)
    {
        cube_mesh[i]                                =                        front_face[i];
        cube_mesh[i +     ARRAY_LENGTH(front_face)] =             x_rotate * front_face[i];
        cube_mesh[i + 2 * ARRAY_LENGTH(front_face)] =  x_rotate * x_rotate * front_face[i];
        cube_mesh[i + 3 * ARRAY_LENGTH(front_face)] = x_rotate.transpose() * front_face[i];
        cube_mesh[i + 4 * ARRAY_LENGTH(front_face)] =             y_rotate * front_face[i];
        cube_mesh[i + 5 * ARRAY_LENGTH(front_face)] = y_rotate.transpose() * front_face[i];
    }

    return cube_mesh;
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

    // Generate cube vao
    {
        glGenBuffers(1, &cube_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

        const Vec3* cube_vertices = generate_cube_mesh();

        glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(Vec3), cube_vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &cube_vao);
        glBindVertexArray(cube_vao);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vec3), (const void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vec3), (const void*)sizeof(Vec3));

        simple_shader = load_shader("shaders/simple_vs.glsl", "shaders/simple_fs.glsl");
    }

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
    Mat3 rotation = Mat3::RotateZ(box.rotation);

    glBindVertexArray(cube_vao);

    GLint loc = glGetUniformLocation(selected_shader, "rotation");
    glUniformMatrix3fv(loc, 1, GL_TRUE, rotation.data);

    loc = glGetUniformLocation(selected_shader, "scale");
    glUniform3fv(loc, 1, box.scale.array());

    loc = glGetUniformLocation(selected_shader, "position");
    glUniform3fv(loc, 1, box.pos.array());

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void present_screen(SDL_Window* window)
{
    SDL_GL_SwapWindow(window);
    sample_screen_size(window);
}

/*
void Renderer::debug_draw_rectangle(Transform2d rect, float r, float g, float b) const
{
    Mat2 rotation = Mat2::Rotation(rect.rotation);

    glBindVertexArray(rect_vao);
    glUseProgram(debug_shader);


    GLint color_loc    = glGetUniformLocation(debug_shader, "color");
    glUniform3f(color_loc, r, g, b);

    GLint position_loc = glGetUniformLocation(debug_shader, "position");
    glUniform2fv(position_loc, 1, rect.pos.array());

    GLint rotation_loc = glGetUniformLocation(debug_shader, "rotation");
    glUniformMatrix2fv(rotation_loc, 1, GL_TRUE, rotation.data);

    GLint scale_loc    = glGetUniformLocation(debug_shader, "scale");
    glUniform2fv(scale_loc, 1, rect.scale.array());

    glDrawArrays(GL_LINE_LOOP, 0, 4);
}
*/

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

}
