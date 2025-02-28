#version 460 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D screen_texture;

uniform float time;

float rand(vec2 co)
{
    return fract(sin(dot(co * time * 0.001, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 grain(vec4 fragColor, vec2 uv)
{
    vec4 color = fragColor;
    float diff = rand(uv) * 0.8;
    color.r += diff;
    color.g += diff;
    color.b += diff;
    return color;
}

void main()
{
    float pixels = 1024.0;
    float dx = 2.0 * (1.0 / pixels);
    float dy = 2.0 * (1.0 / pixels);
    vec2 coord = vec2(dx * floor(uv.x / dx), dy * floor(uv.y / dy));
    frag_color = texture(screen_texture, coord);
    vec4 grain = grain(frag_color, uv);
    frag_color = mix(frag_color, grain, 0.1);
    frag_color = floor(frag_color * 9.5) / 11.0;
}
