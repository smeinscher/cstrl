#version 460 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

out vec2 uv;
out vec4 color;

layout(std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

void main()
{
    gl_Position = projection * view * vec4(a_pos, -1.0, 1.0);
    uv = a_uv;
    color = a_color;
}
