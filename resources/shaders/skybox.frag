#version 460 core
out vec4 frag_color;

in vec3 texture_coords;

uniform samplerCube skybox;

void main()
{
    frag_color = texture(skybox, texture_coords);
}
