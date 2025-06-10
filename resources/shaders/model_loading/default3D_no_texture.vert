#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec3 a_normal;

out vec4 color;
out vec3 normal;
out vec3 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    color = a_color;
    frag_pos = vec3(model * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(model))) * a_normal;
    gl_Position = projection * view * vec4(frag_pos, 1.0);
}
