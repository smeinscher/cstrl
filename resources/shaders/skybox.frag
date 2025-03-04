#version 460 core
out vec4 frag_color;

in vec3 texture_coords;
in vec4 color;

uniform samplerCube skybox;

void main()
{
    frag_color = color * texture(skybox, texture_coords);
    frag_color = floor(frag_color * 10.0) / 10.0;
}
