//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"

#include <stdlib.h>

const char *vertex_shader_source = R"(
    #version 460 core
    layout (location = 0) in vec2 a_pos;
    layout (location = 1) in vec2 a_uv;
    layout (location = 2) in vec4 a_color;

    out vec2 uv;
    out vec4 color;

    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * vec4(a_pos, -1.0, 1.0);
        uv = a_uv;
        color = a_color;
    }
)";
const char *fragment_shader_source = R"(
    #version 460 core

    out vec4 frag_color;

    in vec2 uv;
    in vec4 color;

    uniform sampler2D texture0;

    void main()
    {
        frag_color = color;// * texture(texture0, uv);
    }
)";

cstrl_ui_context *cstrl_ui_init(cstrl_platform_state *platform_state)
{
    cstrl_ui_context *context = malloc(sizeof(cstrl_ui_context));

    context->platform_state = platform_state;

    context->render_data = cstrl_renderer_create_render_data(platform_state);
    context->shader = cstrl_load_shaders_from_source(vertex_shader_source, fragment_shader_source);
    float vertices[] = {10.0f, 320.0f, 210.0f, 10.0f, 10.0f, 10.0f, 10.0f, 320.0f, 210.0f, 10.0f, 210.0f, 320.0f};
    cstrl_renderer_add_positions(context->render_data, vertices, 2, 6);
    float uvs[] = {0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    cstrl_renderer_add_uvs(context->render_data, uvs);
    float colors[24];
    for (int i = 0; i < 6; i++)
    {
        colors[i * 4] = 0.3f;
        colors[i * 4 + 1] = 0.7f;
        colors[i * 4 + 2] = 1.0f;
        colors[i * 4 + 3] = 0.8f;
    }
    cstrl_renderer_add_colors(context->render_data, colors);

    cstrl_set_uniform_mat4(context->shader.program, "projection",
                           cstrl_mat4_ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.1f, 100.0f));
    return context;
}

void cstrl_ui_begin(cstrl_ui_context *context)
{
}

void cstrl_ui_end(cstrl_ui_context *context)
{
    cstrl_use_shader(context->shader);
    cstrl_renderer_draw(context->render_data);
}

void cstrl_ui_shutdown(cstrl_ui_context *context)
{
    free(context);
}
