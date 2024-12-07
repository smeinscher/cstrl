//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"

#include <stdlib.h>

static struct internal_ui_state
{
    int mouse_x;
    int mouse_y;
    bool left_mouse_button_down;

    int total_items;

    int hot_item;
    int active_item;

    da_float positions;
    da_float uvs;
    da_float colors;
} internal_ui_state;

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
    internal_ui_state.total_items = 0;
    cstrl_da_float_init(&internal_ui_state.positions, 12);
    cstrl_da_float_init(&internal_ui_state.uvs, 12);
    cstrl_da_float_init(&internal_ui_state.colors, 24);
    return context;
}

void cstrl_ui_begin(cstrl_ui_context *context)
{
    internal_ui_state.hot_item = -1;
    internal_ui_state.active_item = -1;
    internal_ui_state.total_items = 0;
    cstrl_platform_get_cursor_position(context->platform_state, &internal_ui_state.mouse_x, &internal_ui_state.mouse_y);
    internal_ui_state.left_mouse_button_down =
        cstrl_platform_is_mouse_button_down(context->platform_state, CSTRL_MOUSE_BUTTON_LEFT);
    cstrl_da_float_clear(&internal_ui_state.positions);
    cstrl_da_float_clear(&internal_ui_state.uvs);
    cstrl_da_float_clear(&internal_ui_state.colors);
}

void cstrl_ui_end(cstrl_ui_context *context)
{
    cstrl_renderer_modify_render_attributes(context->render_data, internal_ui_state.positions.array,
                                            internal_ui_state.uvs.array, internal_ui_state.colors.array,
                                            internal_ui_state.positions.size / 2, 0);
    cstrl_use_shader(context->shader);
    cstrl_renderer_draw(context->render_data);
}

void cstrl_ui_shutdown(cstrl_ui_context *context)
{
    cstrl_renderer_free_render_data(context->render_data);
    cstrl_da_float_free(&internal_ui_state.positions);
    cstrl_da_float_free(&internal_ui_state.uvs);
    cstrl_da_float_free(&internal_ui_state.colors);
    free(context);
}

bool cstrl_ui_region_hit(int x, int y, int w, int h)
{
    if (internal_ui_state.mouse_x < x || internal_ui_state.mouse_y < y || internal_ui_state.mouse_x > x + w ||
        internal_ui_state.mouse_y > y + h)
    {
        return false;
    }
    return true;
}

bool cstrl_ui_button(cstrl_ui_context *context, int x, int y, int w, int h)
{
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)x);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)y + (float)h);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)x + (float)w);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)y);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)x);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)y);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)x);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)y + (float)h);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)x + (float)w);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)y);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)x + (float)w);
    cstrl_da_float_push_back(&internal_ui_state.positions, (float)y + (float)h);
    for (int i = 0; i < 12; i++)
    {
        cstrl_da_float_push_back(&internal_ui_state.uvs, 0.0f);
    }
    if (cstrl_ui_region_hit(x, y, w, h))
    {
        internal_ui_state.hot_item = internal_ui_state.total_items++;
        if (internal_ui_state.left_mouse_button_down && internal_ui_state.active_item == -1)
        {
            internal_ui_state.active_item = internal_ui_state.hot_item;
            for (int i = 0; i < 6; i++)
            {
                cstrl_da_float_push_back(&internal_ui_state.colors, 1.0f);
                cstrl_da_float_push_back(&internal_ui_state.colors, 0.4f);
                cstrl_da_float_push_back(&internal_ui_state.colors, 0.4f);
                cstrl_da_float_push_back(&internal_ui_state.colors, 1.0f);
            }
            return true;
        }
        for (int i = 0; i < 6; i++)
        {
            cstrl_da_float_push_back(&internal_ui_state.colors, 0.9f);
            cstrl_da_float_push_back(&internal_ui_state.colors, 0.3f);
            cstrl_da_float_push_back(&internal_ui_state.colors, 0.3f);
            cstrl_da_float_push_back(&internal_ui_state.colors, 0.9f);
        }
        return false;
    }
    for (int i = 0; i < 6; i++)
    {
        cstrl_da_float_push_back(&internal_ui_state.colors, 0.7f);
        cstrl_da_float_push_back(&internal_ui_state.colors, 0.2f);
        cstrl_da_float_push_back(&internal_ui_state.colors, 0.2f);
        cstrl_da_float_push_back(&internal_ui_state.colors, 0.9f);
    }
    return false;
}