#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

out vec2 uv;
out vec4 color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0, 1.0);
    uv = aUV;
    color = aColor;
}
