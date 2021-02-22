#version 330 core

layout (location = 0) in vec3 offset;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out VS_OUT
{
    vec3 world_normal;
    vec3 world_pos;
    vec4 light_coord;
    vec2 uv;
} vs_out;

uniform vec3 position;
uniform mat3 rotation;
uniform vec3 scale;

uniform mat4 camera;
uniform mat4 light;

void main()
{
    vs_out.world_pos = position + rotation * (scale * offset);
    vs_out.world_normal = normalize(rotation * (normal / scale));
    vs_out.light_coord = light * vec4(vs_out.world_pos, 1.0f);
    vs_out.uv = uv;

    gl_Position = camera * vec4(vs_out.world_pos, 1.0f);
}
