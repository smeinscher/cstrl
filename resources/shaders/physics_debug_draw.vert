#version 460 core

layout(location = 0) in vec2 a_pos;
layout(location = 2) in vec4 a_color;

out vec4 color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(a_pos, 1.0, 1.0);
    color = a_color;
}

