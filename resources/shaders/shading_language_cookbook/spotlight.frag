#version 460 core

in vec3 position;
in vec3 normal;

uniform struct spotlight_info
{
    vec3 position;
    vec3 l;
    vec3 ambient;
    vec3 direction;
    float exponent;
    float cutoff;
} spotlight;

uniform struct material_info
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

out vec4 frag_color;

vec3 blinn_phong_spotlight(vec3 pos, vec3 n)
{
    vec3 ambient = spotlight.ambient * material.ambient;
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 s = normalize(spotlight.position - pos);
    float cos_ang = dot(-s, normalize(spotlight.direction));
    float angle = acos(cos_ang);
    float spot_scale = 0.0;
    if (angle >= 0.0 && angle < spotlight.cutoff)
    {
        spot_scale = pow(cos_ang, spotlight.exponent);
        float s_dot_n = max(dot(s, n), 0.0);
        if (s_dot_n > 0.0)
        {
            vec3 v = normalize(-pos.xyz);
            vec3 h = normalize(v + s);
            specular = material.specular * pow(max(dot(h, n), 0.0), material.shininess);
        }
    }
    return ambient + spot_scale * spotlight.l * (diffuse + specular);
}

void main()
{
    frag_color = vec4(blinn_phong_spotlight(position, normalize(normal)), 1.0);
}
