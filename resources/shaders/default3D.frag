#version 460 core

out vec4 frag_color;

in vec2 uv;
in vec4 color;

uniform sampler2D texture1;

void main()
{
    float pixels = 1024.0;
    float dx = 8.0 * (1.0 / pixels);
    float dy = 8.0 * (1.0 / pixels);
    vec2 coord = vec2(dx * floor(uv.x / dx), dy * floor(uv.y / dy));
    frag_color = color * texture(texture1, uv);
    frag_color = floor(frag_color * 14.0) / 9.0;
    if (frag_color.a < 0.1)
    {
        discard;
    }
}
