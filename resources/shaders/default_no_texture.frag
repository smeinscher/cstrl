#version 460 core

out vec4 FragColor;

in vec2 uv;
in vec4 color;

void main()
{
    FragColor = color;
}
