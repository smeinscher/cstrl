#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec3 a_normal;

out vec4 color;
out vec3 frag_position;
out vec3 normal;
out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(a_pos, 1.0);
    frag_position = a_pos.xyz; //vec3(model * vec4(a_pos, 1.0));
    normal = /*mat3(transpose(inverse(model))) * */ a_normal;
    color = a_color;
    uv = a_uv;
}
