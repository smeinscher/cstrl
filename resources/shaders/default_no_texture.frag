#version 460 core

out vec4 frag_color;

in vec2 uv;
in vec4 color;

void main()
{
    frag_color = color;
}
