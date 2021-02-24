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

uniform float diffuse_ratio;
uniform float shininess;

uniform vec3 camera_dir;

uniform bool lit;
uniform bool textured;

void main()
{
    float lighting_factor = 1.0f;
    if (lit)
    {
        vec3 light_vector = fs_in.world_pos - light_pos;

        float intensity = 10.0f / dot(light_vector, light_vector);
        vec3 light_dir = normalize(light_vector);

        float specular_ratio = 1.0f - diffuse_ratio;

        float diffuse = intensity * diffuse_ratio * dot(fs_in.world_normal, -light_dir);
        float specular = intensity * specular_ratio * pow(dot(-camera_dir, light_dir - 2.0f * fs_in.world_normal * dot(fs_in.world_normal, light_dir)), 20.0f);

        float brightness = max(diffuse, 0.0f) + max(specular, 0.0f);

        vec3 light_coord = (fs_in.light_coord.xyz / fs_in.light_coord.w) * 0.5f + 0.5f;
        light_coord.z -= 0.0005f;

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
