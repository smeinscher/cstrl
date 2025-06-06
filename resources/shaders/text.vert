#version 460 core
layout(location = 0) in vec4 vertex;

out vec2 uv;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(vertex.xy, 0.0, 1.0);
    uv = vertex.zw;
}
