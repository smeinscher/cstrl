#version 460 core

out vec4 frag_color;

in vec4 color;
in vec2 frag_tex_coord;

void main()
{
    float dist = length(frag_tex_coord - vec2(0.5));
    if (dist > 0.314)
    {
        discard;
    }
    frag_color = color;
}
