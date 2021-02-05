#version 330 core

layout (location = 0) in vec3 offset;
layout (location = 1) in vec3 normal;

out vec3 color;

uniform vec3 position;
uniform mat3 rotation;
uniform vec3 scale;

uniform mat4 camera;
uniform vec3 light_direction;

void main()
{
    vec3 out_pos = position + rotation * (scale * offset);

    gl_Position = camera * vec4(out_pos, 1.0f);
    color = -dot(rotation * normalize(normal / scale), light_direction) * vec3(1.0f, 1.0f, 1.0f);
}
