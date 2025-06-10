#version 460 core

out vec4 frag_color;

in vec2 uv;
in vec4 color;

uniform sampler2D texture1;

void main()
{
    frag_color = color * texture(texture1, uv);
}
