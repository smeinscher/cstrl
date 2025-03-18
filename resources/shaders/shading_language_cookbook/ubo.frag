#version 460 core

in vec2 uv;

out vec4 frag_color;

layout(binding = 0) uniform BlobSettings
{
    vec4 inner_color;
    vec4 outer_color;
    float radius_inner;
    float radius_outer;
};

void main()
{
    float dx = uv.x - 0.5;
    float dy = uv.y - 0.5;
    float dist = sqrt(dx * dx + dy * dy);
    frag_color = mix(inner_color, outer_color, smoothstep(radius_inner, radius_outer, dist));
}
