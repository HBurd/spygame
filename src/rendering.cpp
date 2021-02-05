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

Vec3 CameraView::pixel_direction(int x, int y, int width, int height) const
{
    // The +0.5f shifts the coordinate to the centre of the pixel
    Vec3 dir = Vec3(x, y, 0.0f) - 0.5f * Vec3(width, height, 0.0f) + Vec3(0.5f, 0.5f, 0.0f);

    // Flip the y axis to point up
    dir.y = -dir.y;

    // Width of the rectangle filling the screen at distance cos(fov/2) from the camera
    float image_width = 2.0f * sinf(0.5f * fov);

    // Scale to the size of this rectangle
    dir *= image_width / width;

    // Move onto this rectangle
    dir.z = -cosf(0.5f * fov);

    // Apply camera rotation
    dir = compute_rotation() * dir;

    return dir;
}

Vec3 CameraView::compute_position() const
{
    return target + compute_rotation() * Vec3(0.0f, 0.0f, distance);
}

Mat3 CameraView::compute_rotation() const
{
    return Mat3::RotateZ(yaw) * Mat3::RotateX(pitch);
}

Renderer::Renderer(SDL_Window* window)
{
    update_screen_size(window);

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

        debug_shader = link_program(
            compile_shader("shaders/debug_vs.glsl", GL_VERTEX_SHADER),
            compile_shader("shaders/debug_fs.glsl", GL_FRAGMENT_SHADER));
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

        simple_shader = link_program(
            compile_shader("shaders/simple_vs.glsl", GL_VERTEX_SHADER),
            compile_shader("shaders/simple_fs.glsl", GL_FRAGMENT_SHADER)
        );
    }
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
        * Mat4(camera.compute_rotation().transpose())
        * Mat4::Translate(-camera.target);
}

void Renderer::present(SDL_Window* window)
{
    SDL_GL_SwapWindow(window);
    update_screen_size(window);
}

void Renderer::debug_draw_rectangle(Transform2d rect, float r, float g, float b) const
{
    Mat2 rotation = Mat2::Rotation(rect.rotation);

    glBindVertexArray(rect_vao);
    glUseProgram(debug_shader);

    GLint camera_loc   = glGetUniformLocation(debug_shader, "camera");
    glUniformMatrix4fv(camera_loc, 1, GL_TRUE, camera_matrix.data);

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

void Renderer::draw_box(Transform3d box)
{
    Mat3 rotation = Mat3::RotateZ(box.rotation);

    glBindVertexArray(cube_vao);
    glUseProgram(simple_shader);

    GLint camera_loc   = glGetUniformLocation(simple_shader, "camera");
    glUniformMatrix4fv(camera_loc, 1, GL_TRUE, camera_matrix.data);

    GLint rotation_loc = glGetUniformLocation(simple_shader, "rotation");
    glUniformMatrix3fv(rotation_loc, 1, GL_TRUE, rotation.data);

    GLint scale_loc = glGetUniformLocation(simple_shader, "scale");
    glUniform3fv(scale_loc, 1, box.scale.array());

    GLint position_loc = glGetUniformLocation(simple_shader, "position");
    glUniform3fv(position_loc, 1, box.pos.array());

    GLint light_direction_loc = glGetUniformLocation(simple_shader, "light_direction");
    glUniform3fv(light_direction_loc, 1, light_direction.normalize().array());

    glDrawArrays(GL_TRIANGLES, 0, 36);
}
