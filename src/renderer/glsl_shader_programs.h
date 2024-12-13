//
// Created by sterling on 11/24/24.
//

#ifndef OPENGL_SHADER_PROGRAMS_H
#define OPENGL_SHADER_PROGRAMS_H

const char *basic_2d_vertex_shader = "\
    #version 460 core\n\
    layout (location = 0) in vec2 aPos;\
    void main()\
    {\
        gl_Position = vec4(aPos, 1.0, 1.0);\
    }";
const char *basic_2d_fragment_shader = "\
    #version 460 core\n\
    out vec4 FragColor;\
    void main()\
    {\
        FragColor = vec4(0.7, 0.4, 0.2, 1.0);\
    }";

const char *basic_2d_color_vertex_shader = "\
    #version 460 core\n\
    layout (location = 0) in vec2 aPos;\
    layout (location = 1) in vec4 aColor;\
    out vec4 color;\
    void main()\
    {\
        gl_Position = vec4(aPos, 1.0, 1.0);\
        color = aColor;\
    }";
const char *basic_2d_color_fragment_shader = "\
    #version 460 core\n\
    out vec4 FragColor;\
    in vec4 color;\
    void main()\
    {\
        FragColor = color;\
    }";

const char *pulsing_triangle_vertex_shader = "\
    #version 460 core\n\
    layout(location = 0) in vec2 aPos;\
    out float t;\
    uniform float time;\
    void main()\
    {\
        gl_Position = vec4(aPos, 1.0, 1.0);\
        t = time;\
    }";
const char *pulsing_triangle_fragment_shader = "\
    #version 460 core\n\
    out vec4 FragColor;\
    in float t;\
    void main()\
    {\
        FragColor = vec4(t, 0.4, 0.2, 1.0);\
    }";

const char *basic_3d_vertex_shader = "\
    #version 460 core\n\
    layout(location = 0) in vec3 aPos;\
    layout(location = 1) in vec2 aUV;\
    out vec2 uv;\
    uniform mat4 view;\
    uniform mat4 projection;\
    void main()\
    {\
        gl_Position = projection * view * vec4(aPos, 1.0);\
        uv = aUV;\
    }";
const char *basic_3d_fragment_shader = "\
    #version 460 core\n\
    out vec4 FragColor;\
    in vec2 uv;\
    uniform sampler2D texture1;\
    void main()\
    {\
        FragColor = texture(texture1, uv) * vec4(0.7, 0.4, 0.2, 1.0);\
    }";

const char *opengl_es_vertex_shader = "\
    #version 300 es\n\
    layout (location = 0) in vec2 a_pos;\
    void main()\
    {\
        gl_Position = vec4(a_pos, -1.0, 1.0);\
    }";
const char *opengl_es_fragment_shader = "\
    #version 300 es\n\
    prevision mediump float;\
    out vec4 frag_color;\
    void main()\
    {\
        frag_color = vec4(0.7, 0.5, 0.2, 1.0);\
    }";

#endif // OPENGL_SHADER_PROGRAMS_H
