#version 460 core

in vec3 pos;
in vec3 normal;

out vec4 frag_color;

uniform struct light_info
{
    vec4 position;
    vec3 ambient;
    vec3 l;
} light;

uniform struct material_info
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

vec3 blinn_phong_model(vec3 position, vec3 n)
{
    vec3 ambient = light.ambient * material.ambient;
    vec3 s = normalize(light.position.xyz - position);
    float s_dot_n = max(dot(s, n), 0.0);
    vec3 diffuse = material.diffuse * s_dot_n;
    vec3 specular = vec3(0.0);
    if (s_dot_n > 0.0)
    {
        vec3 v = normalize(-position.xyz);
        vec3 h = normalize(v + s);
        specular = material.specular * pow(max(dot(h, n), 0.0), material.shininess);
    }
    return ambient + light.l * (diffuse + specular);
}

void main()
{
    frag_color = vec4(blinn_phong_model(pos, normal), 1.0);
}
