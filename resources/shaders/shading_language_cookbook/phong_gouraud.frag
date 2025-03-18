#version 460 core

in vec3 light_intensity;
layout(location = 0) out vec4 frag_color;

void main()
{
    frag_color = vec4(light_intensity, 1.0);
}
