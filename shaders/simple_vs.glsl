#version 300 es
precision highp float;

layout (location = 0) in vec3 offset;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 vs_out_world_normal;
out vec3 vs_out_world_pos;
out vec4 vs_out_light_coord;
out vec2 vs_out_uv;

uniform vec3 position;
uniform mat3 rotation;
uniform vec3 scale;

uniform mat4 camera;
uniform mat4 light;

void main()
{
    vs_out_world_pos = position + rotation * (scale * offset);
    vs_out_world_normal = normalize(rotation * (normal / scale));
    vs_out_light_coord = light * vec4(vs_out_world_pos, 1.0f);
    vs_out_uv = uv;

    gl_Position = camera * vec4(vs_out_world_pos, 1.0f);
}
