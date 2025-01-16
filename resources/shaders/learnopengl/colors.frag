#version 460 core

out vec4 frag_color;

in vec3 frag_position;
in vec3 normal;
in vec2 uv;

uniform vec3 view_position;

struct material_t
{
    sampler2D diffuse;
    sampler2D specular;
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

void main()
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, uv));

    vec3 norm = normalize(normal);
    vec3 light_direction = normalize(light.position - frag_position);
    float diff = max(dot(norm, light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, uv));

    vec3 view_direction = normalize(view_position - frag_position);
    vec3 reflect_direction = reflect(-light_direction, norm);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, uv));

    vec3 result = ambient + diffuse + specular;
    frag_color = vec4(result, 1.0);
}
