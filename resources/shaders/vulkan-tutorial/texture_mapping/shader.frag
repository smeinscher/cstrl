#version 450

layout (location = 0) in vec3 color;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec4 out_color;

layout (binding = 1) uniform sampler2D texture0;

void main()
{
    out_color = texture(texture0, uv);
}
