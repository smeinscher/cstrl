#version 460 core
layout(location = 0) in vec3 a_pos;
layout(location = 2) in vec4 a_color;

out vec3 texture_coords;
out vec4 color;

layout(std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

void main()
{
    vec4 pos = projection * mat4(mat3(view)) * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
    texture_coords = a_pos;
    color = a_color;
}
