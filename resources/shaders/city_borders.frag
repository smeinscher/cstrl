#version 460 core

uniform vec3 city_centers[100];
uniform int team[100];
uniform float weights[100];
uniform float influence_strength[100];
uniform float influence_radius[100];
uniform int cities_count;
uniform sampler2D noise_texture;

out vec4 frag_color;

in vec3 frag_position;

float get_noise(vec3 pos)
{
    return texture(noise_texture, pos.xz * 0.2).r;
}

float compute_influence(vec3 city_pos, float city_strength, float max_distance, vec3 position)
{
    float distance = length(city_pos - position);
    if (distance > max_distance)
    {
        return 0.0;
    }
    return city_strength / (distance * distance + 0.01);
}

void main()
{
    int dominant_team = -1;
    float max_influence = 0.0;
    float second_max_influence = 0.0;

    for (int i = 0; i < cities_count; i++)
    {
        float influence = compute_influence(city_centers[i], influence_strength[i], influence_radius[i], frag_position);

        if (influence > max_influence)
        {
            second_max_influence = max_influence;
            max_influence = influence;
            dominant_team = team[i];
        }
        else if (influence > second_max_influence)
        {
            second_max_influence = influence;
        }
    }

    float border_factor = clamp((max_influence - second_max_influence) * 8.0, 0.0, 1.0);

    vec3 colors[8] = vec3[8](vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(0.3, 0.3, 0.3));
    vec3 color = colors[dominant_team];

    frag_color = vec4(color, 0.1);
}
