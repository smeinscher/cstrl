#version 460 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D text;
uniform vec4 text_color;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, uv).r);
    frag_color = text_color * sampled;
}

