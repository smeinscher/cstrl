#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 3) in vec3 a_normal;

out vec3 position;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    normal = a_normal;
    position = (projection * view * model * vec4(a_pos, 1.0)).xyz;

    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
