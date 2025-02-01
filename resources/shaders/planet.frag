#version 460 core

out vec4 frag_color;

in vec3 frag_position;
in vec2 uv;
in vec3 normal;
in vec4 color;

uniform vec3 view_position;

struct material_t
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform material_t material;

struct light_t
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform light_t light;

uniform sampler2D texture0;

void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(normal);
    vec3 light_direction = normalize(light.position - frag_position);
    float diff = max(dot(norm, light_direction), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    vec3 view_direction = normalize(view_position - frag_position);
    vec3 reflect_direction = reflect(-light_direction, norm);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    frag_color = vec4(result, 1.0) * texture(texture0, uv);
}
