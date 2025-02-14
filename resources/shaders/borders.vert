#version 460

layout(location = 0) in vec3 a_control_point;
layout(location = 2) in vec4 a_color;

out vec4 color_tesc;

void main()
{
    gl_Position = vec4(a_control_point, 1.0);
    color_tesc = a_color;
}
