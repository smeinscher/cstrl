#version 460 core
layout(location = 0) in vec3 a_pos;

out vec3 texture_coords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 pos = projection * view * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
    texture_coords = a_pos;
}
