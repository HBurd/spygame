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
uniform bool is_directional;
uniform vec3 light_direction;
uniform float intensity;

uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float shininess;

uniform vec3 camera_pos;

uniform bool lit;
uniform bool textured;

void main()
{
    vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
    if (lit)
    {
        vec3 light_vector;
        if (is_directional)
        {
            light_vector = light_direction;
        }
        else
        {
            light_vector = fs_in.world_pos - light_pos;
        }

        float light_per_area = intensity / dot(light_vector, light_vector);
        vec3 light_dir = normalize(light_vector);

        float diffuse = dot(fs_in.world_normal, -light_dir);

        vec3 reflected_ray = light_dir - 2.0f * fs_in.world_normal * dot(fs_in.world_normal, light_dir);

        vec3 camera_dir = normalize(fs_in.world_pos - camera_pos);

        float specular_dot = dot(-camera_dir, reflected_ray);
        float specular;

        // The following statements approximate specular = pow(max(specular_dot, 0.0f), shininess)
        // (but not very well)
        if (shininess >= 4.0f)
        {
            specular = max(0.0f, 1.0f - (shininess / 4.0f) * (1.0f - specular_dot));
            specular = specular * specular * specular * specular;
        }
        else if (shininess >= 2.0f)
        {

            specular = max(0.0f, 1.0f - (shininess / 2.0f) * (1.0f - specular_dot));
            specular = specular * specular;
        }
        else
        {
            specular = max(0.0f, 1.0f - shininess * (1.0f - specular_dot));
        }

        vec3 diffuse_texture = vec3(1.0f, 1.0f, 1.0f);
        if (textured)
        {
            diffuse_texture = texture(color_texture, fs_in.uv).rgb;
        }

        vec3 final_color = light_per_area * (diffuse_texture * diffuse_color * max(diffuse, 0.0f) + specular_color * max(specular, 0.0f));

        vec3 light_coord = (fs_in.light_coord.xyz / fs_in.light_coord.w) * 0.5f + 0.5f;
        light_coord.z -= 0.0005f;

        if (light_coord.x >= 0.0f && light_coord.x <= 1.0f && light_coord.y >= 0.0f && light_coord.y <= 1.0f)
        {
            light_color = texture(light_depth, light_coord) * final_color;
        }
        else
        {
            light_color = final_color;
        }
    }
    else if (textured)
    {
        light_color = texture(color_texture, fs_in.uv).rgb;
    }

    frag_color = vec4(light_color, 1.0f);
}
