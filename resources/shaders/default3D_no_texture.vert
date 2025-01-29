#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec3 a_normal;

out vec4 color;
out vec3 frag_position;
out vec3 normal;
out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    frag_position = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * a_normal;
    color = aColor;
    uv = aUV;
}
