#version 460 core

uniform vec3 city_centers[100];
uniform int team[100];
uniform float weights[100];
uniform float influence_radius[100];
uniform int cities_count;
uniform sampler2D noise_texture;

out vec4 frag_color;

in vec3 frag_position;

float get_noise(vec3 pos)
{
    return texture(noise_texture, pos.xz * 0.2).r;
}

void main()
{
    float min_distance = 1e9;
    int closest = -1;
    float strongest_influence = 0.0;

    for (int i = 0; i < cities_count; i++)
    {
        float dist = distance(frag_position, city_centers[i]);

        float noise_factor = get_noise(frag_position) * 0.5 + 0.5;

        float max_radius = influence_radius[i]; // * noise_factor;

        if (dist < max_radius)
        {
            float influence = smoothstep(max_radius * 0.9, max_radius, dist); //1.0 - (dist / influence_radius[i]);
            strongest_influence += influence;
            closest = i;
        }
    }

    if (closest == -1)
    {
        frag_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    vec3 colors[8] = vec3[8](vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(0.3, 0.3, 0.3));
    vec4 color = vec4(colors[team[closest]], 0.1); //mix(vec4(colors[team[closest]], 0.1), vec4(colors[team[closest]], 0.02), strongest_influence);

    frag_color = color;
}
