#version 330 core

layout (location = 0) in vec3 offset;

uniform vec3 position;
uniform mat3 rotation;
uniform vec3 scale;

uniform mat4 light;

void main()
{
    vec3 out_pos = position + rotation * (scale * offset);
    gl_Position = light * vec4(out_pos, 1.0f);
}
