#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 2) in vec4 a_color;

out vec4 color;

layout(std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

void main()
{
    gl_Position = projection * view * vec4(a_pos, 1.0);
    color = a_color;
}
