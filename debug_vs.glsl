#version 330 core

layout (location = 0) in vec2 vertex_position;

uniform vec2 dimensions;

uniform vec2 position;
uniform vec2 scale;
uniform mat2 rotation;

void main()
{
    vec2 out_position = position + rotation * vec2(scale.x * vertex_position.x, scale.y * vertex_position.y);
    out_position.x /= 0.5f * dimensions.x;
    out_position.y /= 0.5f * dimensions.y;

    gl_Position = vec4(out_position, 0.0f, 1.0f);
}
