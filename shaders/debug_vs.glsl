#version 300 es
precision highp float;

layout (location = 0) in vec2 vertex_position;

uniform vec2 position;
uniform vec2 scale;
uniform mat2 rotation;

uniform mat4 camera;

void main()
{
    vec2 out_position = position + rotation * vec2(scale.x * vertex_position.x, scale.y * vertex_position.y);

    gl_Position = camera * vec4(out_position, 0.0f, 1.0f);
}
