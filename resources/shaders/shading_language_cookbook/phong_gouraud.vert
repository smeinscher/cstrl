#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 3) in vec3 a_normal;

out vec3 light_intensity;

uniform struct light_info
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

uniform struct material_info
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 cam_coords = view * vec4(a_pos, 1.0);
    vec3 s = normalize(vec3(light.position - cam_coords));

    vec3 ambient = light.ambient * material.ambient;
    float s_dot_n = max(dot(s, a_normal), 0.0);
    vec3 diffuse = light.diffuse * material.diffuse * s_dot_n;
    vec3 specular = vec3(0.0);
    if (s_dot_n > 0.0)
    {
        vec3 v = normalize(-cam_coords.xyz);
        vec3 r = reflect(-s, a_normal);
        specular = light.specular * material.specular * pow(max(dot(r, v), 0.0), material.shininess);
    }

    light_intensity = ambient + diffuse + specular;

    gl_Position = projection * view * vec4(a_pos, 1.0);
}
