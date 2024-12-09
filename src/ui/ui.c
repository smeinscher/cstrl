//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include "glad/glad.h"
#include "log.c/log.h"
#include "stb/stb_image_write.h"
#include "stb/stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct internal_ui_state
{
    int mouse_x;
    int mouse_y;
    bool left_mouse_button_down;
    int left_mouse_button_pressed_x;
    int left_mouse_button_pressed_y;

    int total_items;

    int hot_item;
    int active_item;

    da_float positions;
    da_float uvs;
    da_float colors;

    da_float font_positions;
    da_float font_uvs;
    da_float font_colors;

    stbtt_bakedchar *cdata;
    stbtt_packedchar *char_data;
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
        frag_color = color * texture(texture0, uv);
    }
)";

const char *font_vertex_shader_source = R"(
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
const char *font_fragment_shader_source = R"(
    #version 460 core

    out vec4 frag_color;

    in vec2 uv;
    in vec4 color;

    uniform sampler2D texture0;

    void main()
    {
        frag_color = vec4(color.rgb, texture(texture0, uv).r);
    }
)";

static const int FONT_SIZE = 64;

cstrl_ui_context *cstrl_ui_init(cstrl_platform_state *platform_state)
{
    cstrl_ui_context *context = malloc(sizeof(cstrl_ui_context));

    context->platform_state = platform_state;

    context->render_data = cstrl_renderer_create_render_data(platform_state);
    context->shader = cstrl_load_shaders_from_source(vertex_shader_source, fragment_shader_source);
    context->font_render_data = cstrl_renderer_create_render_data(platform_state);
    context->font_shader = cstrl_load_shaders_from_source(font_vertex_shader_source, font_fragment_shader_source);
    float vertices[] = {10.0f, 320.0f, 210.0f, 10.0f, 10.0f, 10.0f, 10.0f, 320.0f, 210.0f, 10.0f, 210.0f, 320.0f};
    float uvs[] = {0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    float colors[24];
    for (int i = 0; i < 6; i++)
    {
        colors[i * 4] = 0.3f;
        colors[i * 4 + 1] = 0.7f;
        colors[i * 4 + 2] = 1.0f;
        colors[i * 4 + 3] = 0.8f;
    }
    cstrl_renderer_add_positions(context->render_data, vertices, 2, 6);
    cstrl_renderer_add_uvs(context->render_data, uvs);
    cstrl_renderer_add_colors(context->render_data, colors);
    cstrl_renderer_add_positions(context->font_render_data, vertices, 2, 6);
    cstrl_renderer_add_uvs(context->font_render_data, uvs);
    cstrl_renderer_add_colors(context->font_render_data, colors);

    mat4 projection = cstrl_mat4_ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.1f, 100.0f);
    cstrl_set_uniform_mat4(context->shader.program, "projection", projection);
    cstrl_set_uniform_mat4(context->font_shader.program, "projection", projection);
    internal_ui_state.total_items = 0;
    cstrl_da_float_init(&internal_ui_state.positions, 12);
    cstrl_da_float_init(&internal_ui_state.uvs, 12);
    cstrl_da_float_init(&internal_ui_state.colors, 24);
    cstrl_da_float_init(&internal_ui_state.font_positions, 12);
    cstrl_da_float_init(&internal_ui_state.font_uvs, 12);
    cstrl_da_float_init(&internal_ui_state.font_colors, 24);

    // The real mvp https://sinf.org/opengl-text-using-stb_truetype/
    long size;
    unsigned char *font_buffer;
    FILE *font_file = fopen("../resources/fonts/NocturneSerif-Regular.ttf", "rb");
    // FILE *font_file = fopen("C:\\Windows\\Fonts\\times.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = malloc(size);

    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    stbtt_pack_context pack_context;
    internal_ui_state.char_data = calloc(126, sizeof(stbtt_packedchar));
    unsigned char *pixels = calloc(512 * 512, sizeof(char));
    stbtt_PackBegin(&pack_context, pixels, 512, 512, 512, 1, NULL);

    stbtt_PackFontRange(&pack_context, font_buffer, 0, FONT_SIZE, 0, 125, internal_ui_state.char_data);
    stbtt_PackEnd(&pack_context);

    context->font_texture = cstrl_texture_generate_from_bitmap(pixels, 512, 512);

    context->texture = cstrl_texture_generate_from_path("../resources/textures/wall.jpg");

    return context;
}

void cstrl_ui_begin(cstrl_ui_context *context)
{
    internal_ui_state.hot_item = -1;
    internal_ui_state.active_item = -1;
    internal_ui_state.total_items = 0;
    cstrl_platform_get_cursor_position(context->platform_state, &internal_ui_state.mouse_x, &internal_ui_state.mouse_y);
    bool previous_left_mouse_button_state = internal_ui_state.left_mouse_button_down;
    internal_ui_state.left_mouse_button_down =
        cstrl_platform_is_mouse_button_down(context->platform_state, CSTRL_MOUSE_BUTTON_LEFT);
    if (!internal_ui_state.left_mouse_button_down)
    {
        internal_ui_state.left_mouse_button_pressed_x = -1;
        internal_ui_state.left_mouse_button_pressed_y = -1;
    }
    else if (!previous_left_mouse_button_state)
    {
        internal_ui_state.left_mouse_button_pressed_x = internal_ui_state.mouse_x;
        internal_ui_state.left_mouse_button_pressed_y = internal_ui_state.mouse_y;
    }
    cstrl_da_float_clear(&internal_ui_state.positions);
    cstrl_da_float_clear(&internal_ui_state.uvs);
    cstrl_da_float_clear(&internal_ui_state.colors);
    cstrl_da_float_clear(&internal_ui_state.font_positions);
    cstrl_da_float_clear(&internal_ui_state.font_uvs);
    cstrl_da_float_clear(&internal_ui_state.font_colors);
}

void cstrl_ui_end(cstrl_ui_context *context)
{
    if (internal_ui_state.total_items < 1)
    {
        return;
    }

    cstrl_renderer_modify_render_attributes(context->render_data, internal_ui_state.positions.array,
                                            internal_ui_state.uvs.array, internal_ui_state.colors.array,
                                            internal_ui_state.positions.size / 2);

    cstrl_renderer_modify_render_attributes(context->font_render_data, internal_ui_state.font_positions.array,
                                            internal_ui_state.font_uvs.array, internal_ui_state.font_colors.array,
                                            internal_ui_state.font_positions.size / 2);

    glDisable(GL_DEPTH_TEST);
    cstrl_texture_bind(context->texture);
    cstrl_use_shader(context->shader);
    cstrl_renderer_draw(context->render_data);
    cstrl_texture_bind(context->font_texture);
    cstrl_use_shader(context->font_shader);
    cstrl_renderer_draw(context->font_render_data);
    glEnable(GL_DEPTH_TEST);
}

void cstrl_ui_shutdown(cstrl_ui_context *context)
{
    cstrl_renderer_free_render_data(context->render_data);
    cstrl_da_float_free(&internal_ui_state.positions);
    cstrl_da_float_free(&internal_ui_state.uvs);
    cstrl_da_float_free(&internal_ui_state.colors);
    cstrl_renderer_free_render_data(context->font_render_data);
    cstrl_da_float_free(&internal_ui_state.font_positions);
    cstrl_da_float_free(&internal_ui_state.font_uvs);
    cstrl_da_float_free(&internal_ui_state.font_colors);
    free(context);
}

bool cstrl_ui_region_hit(int test_x, int test_y, int object_x, int object_y, int object_width, int object_height)
{
    if (test_x < object_x || test_y < object_y || test_x > object_x + object_width || test_y > object_y + object_height)
    {
        return false;
    }
    return true;
}

bool cstrl_ui_button(cstrl_ui_context *context, int x, int y, int w, int h)
{
    internal_ui_state.total_items++;

    float scale = ((float)h / 2.5f) / (float)FONT_SIZE;
    float resolution[2] = {800, 600};
    float position[2] = {(float)x, (float)y + (float)h / 2.0f};

    const char *text = "Hello World!";
    for (int i = 0; i < 12; i++)
    {
        stbtt_packedchar c = internal_ui_state.char_data[text[i]];

        // log_debug("%c: %f, %f, %f, %f, %f", text[i], c.xoff, c.yoff, c.xadvance, c.xoff2, c.yoff2);

        float u0 = (float)c.x0 / 512.0f;
        float v0 = (float)c.y0 / 512.0f;
        float u1 = (float)c.x1 / 512.0f;
        float v1 = (float)c.y1 / 512.0f;

        float size[2] = {(c.xoff2 - c.xoff) * scale, (c.yoff2 - c.yoff) * scale};

        float render_position[2] = {position[0] + c.xoff * scale, position[1] - (c.yoff2 - c.yoff) * scale};

        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[0]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[1] + size[1]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[0] + size[0]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[1]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[0]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[1]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[0]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[1] + size[1]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[0] + size[0]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[1]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[0] + size[0]);
        cstrl_da_float_push_back(&internal_ui_state.font_positions, render_position[1] + size[1]);

        cstrl_da_float_push_back(&internal_ui_state.font_uvs, u0);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, v1);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, u1);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, v0);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, u0);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, v0);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, u0);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, v1);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, u1);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, v0);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, u1);
        cstrl_da_float_push_back(&internal_ui_state.font_uvs, v1);

        for (int j = 0; j < 6; j++)
        {
            cstrl_da_float_push_back(&internal_ui_state.font_colors, 1.0f);
            cstrl_da_float_push_back(&internal_ui_state.font_colors, 1.0f);
            cstrl_da_float_push_back(&internal_ui_state.font_colors, 1.0f);
            cstrl_da_float_push_back(&internal_ui_state.font_colors, 1.0f);
        }

        position[0] += c.xadvance * scale;
    }

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

    if (cstrl_ui_region_hit(internal_ui_state.mouse_x, internal_ui_state.mouse_y, x, y, w, h))
    {
        internal_ui_state.hot_item = internal_ui_state.total_items - 1;
        if (internal_ui_state.left_mouse_button_down && internal_ui_state.active_item == -1 &&
            cstrl_ui_region_hit(internal_ui_state.left_mouse_button_pressed_x,
                                internal_ui_state.left_mouse_button_pressed_y, x, y, w, h))
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