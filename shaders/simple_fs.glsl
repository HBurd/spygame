#version 330 core

out vec4 frag_color;

in VS_OUT
{
    vec3 world_normal;
    vec3 world_pos;
    vec4 light_coord;
    vec2 uv;
} fs_in;

uniform sampler2DShadow light_depth;
uniform sampler2D color_texture;

uniform vec3 light_pos;

uniform bool lit;
uniform bool textured;

void main()
{
    vec3 light_vector = fs_in.world_pos - light_pos;
    float brightness = -10 * dot(fs_in.world_normal, normalize(light_vector)) / dot(light_vector, light_vector);
    vec3 light_coord = (fs_in.light_coord.xyz / fs_in.light_coord.w) * 0.5f + 0.5f;
    light_coord.z -= 0.0005f;

    float lighting_factor = 1.0f;
    if (lit)
    {
        if (light_coord.x >= 0.0f && light_coord.x <= 1.0f && light_coord.y >= 0.0f && light_coord.y <= 1.0f)
        {
            lighting_factor = texture(light_depth, light_coord) * brightness;
        }
        else
        {
            lighting_factor = brightness;
        }
    }

    vec3 color = vec3(1.0f, 1.0f, 1.0f);
    if (textured)
    {
        color = texture(color_texture, fs_in.uv).rgb;
    }

    frag_color = vec4(lighting_factor * color, 1.0f);
}
