#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 3) in vec3 a_normal;

out vec3 color;

uniform struct light_info
{
    vec4 position;
    vec3 ambient;
    vec3 l;
} lights[5];

uniform struct material_info
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

uniform mat4 view;
uniform mat4 projection;

vec3 phong_model(int light, vec3 position, vec3 n)
{
    vec3 ambient = lights[light].ambient * material.ambient;
    vec3 s = normalize(lights[light].position.xyz - position);
    float s_dot_n = max(dot(s, n), 0.0);
    vec3 diffuse = material.diffuse * s_dot_n;
    vec3 specular = vec3(0.0);
    if (s_dot_n > 0.0)
    {
        vec3 v = normalize(-position.xyz);
        vec3 r = reflect(-s, n);
        specular = material.specular * pow(max(dot(r, v), 0.0), material.shininess);
    }

    return ambient + lights[light].l * (diffuse + specular);
}

void main()
{
    vec3 cam_position = (view * vec4(a_pos, 1.0)).xyz;
    color = vec3(0.0);
    for (int i = 0; i < 5; i++)
    {
        color += phong_model(i, cam_position, a_normal);
    }
    gl_Position = projection * view * vec4(a_pos, 1.0);
}
