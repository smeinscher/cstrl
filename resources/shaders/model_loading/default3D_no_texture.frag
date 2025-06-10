#version 460 core

out vec4 frag_color;

in vec4 color;
in vec3 normal;
in vec3 frag_pos;

uniform vec3 light_pos;
uniform vec3 view_pos;

void main()
{
    vec3 light_color = vec3(1.0, 1.0, 1.0);
    vec3 ambient = 0.2 * light_color;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = 0.5 * spec * light_color;

    vec3 result = (ambient + diffuse + specular);
    frag_color = color * vec4(result, 1.0);
}
