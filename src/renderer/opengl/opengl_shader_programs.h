//
// Created by sterling on 11/24/24.
//

#ifndef OPENGL_SHADER_PROGRAMS_H
#define OPENGL_SHADER_PROGRAMS_H

const char *basic_vertex_shader = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
    void main()
    {
        gl_Position = vec4(aPos, 1.0, 1.0);
    })";
const char *basic_fragment_shader = R"(
    #version 460 core
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(0.7, 0.4, 0.2, 1.0);
    }
)";

const char *basic_color_vertex_shader = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec4 aColor;
    out vec4 color;
    void main()
    {
        gl_Position = vec4(aPos, 1.0, 1.0);
        color = aColor;
    })";
const char *basic_color_fragment_shader = R"(
    #version 460 core
    out vec4 FragColor;
    in vec4 color;
    void main()
    {
        FragColor = color;
    }
)";

const char *pulsing_triangle_vertex_shader = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
    out float t;
    uniform float time;
    void main()
    {
        gl_Position = vec4(aPos, 1.0, 1.0);
        t = time;
    })";
const char *pulsing_triangle_fragment_shader = R"(
    #version 460 core
    out vec4 FragColor;
    in float t;
    void main()
    {
        FragColor = vec4(t, 0.4, 0.2, 1.0);
    }
)";

#endif // OPENGL_SHADER_PROGRAMS_H
