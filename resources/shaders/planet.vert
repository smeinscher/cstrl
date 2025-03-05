#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 3) in vec3 a_normal;
layout(location = 4) in vec3 a_tangent;
layout(location = 5) in vec3 a_bitangent;

out vec3 frag_position;
out vec3 normal;
out mat3 tbn;

layout(std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

vec3 compute_tangent(vec3 n)
{
    vec3 up = abs(n.y) > 0.99 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    return normalize(cross(up, n));
}

vec3 compute_bitangent(vec3 n, vec3 t)
{
    return normalize(cross(n, t));
}

void main()
{
    gl_Position = projection * view * vec4(a_pos, 1.0);
    frag_position = a_pos.xyz;
    normal = a_normal;
    vec3 t = compute_tangent(a_normal);
    tbn = mat3(t, compute_bitangent(a_normal, t), a_normal);
}
