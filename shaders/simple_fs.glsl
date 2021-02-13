#version 330 core

out vec4 frag_color;

in VS_OUT
{
    vec3 world_normal;
    vec3 world_pos;
    vec4 light_coord;
} fs_in;

uniform sampler2DShadow light_depth;
uniform vec3 light_pos;

void main()
{
    vec3 light_vector = fs_in.world_pos - light_pos;
    float brightness = -10 * dot(fs_in.world_normal, normalize(light_vector)) / dot(light_vector, light_vector);
    vec3 light_coord = (fs_in.light_coord.xyz / fs_in.light_coord.w) * 0.5f + 0.5f;
    light_coord.z -= 0.0005f;
    frag_color = vec4(texture(light_depth, light_coord) * brightness * vec3(1.0f, 1.0f, 1.0f), 1.0f);
}
