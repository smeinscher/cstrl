//
// Created by sterling on 12/5/24.
//

#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#if defined(CSTRL_PLATFORM_ANDROID)
#include <glad/gles3/glad.h>
#else
#include "glad/glad.h"
#endif
#include "log.c/log.h"
#include "stb/stb_image_write.h"
#include "stb/stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct cstrl_ui_elements
{
    int element_count;
    da_int ids;
    // x0, y0, x1, y1
    da_int screen_coords;
    // r, g, b, a
    da_float colors;
    da_string titles;
    da_int parent_index;
    da_int order_priority;
} cstrl_ui_elements;

typedef struct cstrl_ui_internal_state
{
    cstrl_platform_state *platform_state;
    cstrl_render_data *render_data;
    cstrl_shader shader;
    cstrl_texture texture;
    cstrl_render_data *font_render_data;
    cstrl_shader font_shader;
    cstrl_texture font_texture;

    cstrl_ui_elements elements;
    cstrl_ui_elements element_cache;
    da_int element_render_order;

    int dragged_element_id;

    int mouse_x;
    int mouse_y;
    int prev_mouse_x;
    int prev_mouse_y;
    bool left_mouse_button_down;
    bool left_mouse_button_processed;
    int left_mouse_button_pressed_x;
    int left_mouse_button_pressed_y;

    int hot_item;
    int active_item;

    cstrl_key key_pressed;
    bool key_press_processed;

    da_float positions;
    da_float uvs;
    da_float colors;

    da_float font_positions;
    da_float font_uvs;
    da_float font_colors;

    stbtt_packedchar *char_data;

    da_int parent_stack;

} cstrl_ui_internal_state;

const char *vertex_shader_source = "                            \
    #version 460 core\n                                         \
    layout (location = 0) in vec2 a_pos;                        \
    layout (location = 1) in vec2 a_uv;                         \
    layout (location = 2) in vec4 a_color;                      \
                                                                \
    out vec2 uv;                                                \
    out vec4 color;                                             \
                                                                \
    uniform mat4 projection;                                    \
                                                                \
    void main()                                                 \
    {                                                           \
        gl_Position = projection * vec4(a_pos, -1.0, 1.0);      \
        uv = a_uv;                                              \
        color = a_color;                                        \
    }";

const char *fragment_shader_source = "                          \
    #version 460 core\n                                         \
                                                                \
    out vec4 frag_color;                                        \
                                                                \
    in vec2 uv;                                                 \
    in vec4 color;                                              \
                                                                \
    uniform sampler2D texture0;                                 \
                                                                \
    void main()                                                 \
    {                                                           \
        frag_color = color * texture(texture0, uv);             \
    }";

const char *font_vertex_shader_source = "                       \
    #version 460 core\n                                         \
    layout (location = 0) in vec2 a_pos;                        \
    layout (location = 1) in vec2 a_uv;                         \
    layout (location = 2) in vec4 a_color;                      \
                                                                \
    out vec2 uv;                                                \
    out vec4 color;                                             \
                                                                \
    uniform mat4 projection;                                    \
                                                                \
    void main()                                                 \
    {                                                           \
        gl_Position = projection * vec4(a_pos, -1.0, 1.0);      \
        uv = a_uv;                                              \
        color = a_color;                                        \
    }";
const char *font_fragment_shader_source = "                     \
    #version 460 core\n                                         \
                                                                \
    out vec4 frag_color;                                        \
                                                                \
    in vec2 uv;                                                 \
    in vec4 color;                                              \
                                                                \
    uniform sampler2D texture0;                                 \
                                                                \
    void main()                                                 \
    {                                                           \
        frag_color = vec4(color.rgb, texture(texture0, uv).r);  \
    }";

#define FONT_SIZE 32

CSTRL_API cstrl_ui_context *cstrl_ui_init(cstrl_platform_state *platform_state)
{
    cstrl_ui_context *context = malloc(sizeof(cstrl_ui_context));

    context->internal_ui_state = malloc(sizeof(cstrl_ui_internal_state));

    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    ui_state->platform_state = platform_state;

    ui_state->render_data = cstrl_renderer_create_render_data();
    ui_state->shader = cstrl_load_shaders_from_source(vertex_shader_source, fragment_shader_source);
    ui_state->font_render_data = cstrl_renderer_create_render_data();
    ui_state->font_shader = cstrl_load_shaders_from_source(font_vertex_shader_source, font_fragment_shader_source);
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
    cstrl_renderer_add_positions(ui_state->render_data, vertices, 2, 6);
    cstrl_renderer_add_uvs(ui_state->render_data, uvs);
    cstrl_renderer_add_colors(ui_state->render_data, colors);
    cstrl_renderer_add_positions(ui_state->font_render_data, vertices, 2, 6);
    cstrl_renderer_add_uvs(ui_state->font_render_data, uvs);
    cstrl_renderer_add_colors(ui_state->font_render_data, colors);

    mat4 projection = cstrl_mat4_ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.1f, 100.0f);
    cstrl_set_uniform_mat4(ui_state->shader.program, "projection", projection);
    cstrl_set_uniform_mat4(ui_state->font_shader.program, "projection", projection);
    cstrl_da_float_init(&ui_state->positions, 12);
    cstrl_da_float_init(&ui_state->uvs, 12);
    cstrl_da_float_init(&ui_state->colors, 24);
    cstrl_da_float_init(&ui_state->font_positions, 12);
    cstrl_da_float_init(&ui_state->font_uvs, 12);
    cstrl_da_float_init(&ui_state->font_colors, 24);

    // The real mvp https://sinf.org/opengl-text-using-stb_truetype/
    FILE *font_file = fopen("resources/fonts/NocturneSerif-Regular.ttf", "rb");
    // FILE *font_file = fopen("C:\\Windows\\Fonts\\times.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    long size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    unsigned char *font_buffer = malloc(size);

    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    stbtt_pack_context pack_context;
    ui_state->char_data = calloc(126, sizeof(stbtt_packedchar));
    unsigned char *pixels = calloc(512 * 512, sizeof(char));
    stbtt_PackBegin(&pack_context, pixels, 512, 512, 512, 1, NULL);

    stbtt_PackFontRange(&pack_context, font_buffer, 0, FONT_SIZE, 0, 125, ui_state->char_data);
    stbtt_PackEnd(&pack_context);

    ui_state->font_texture = cstrl_texture_generate_from_bitmap(pixels, 512, 512);

    free(pixels);
    free(font_buffer);

    ui_state->texture = cstrl_texture_generate_from_path("resources/textures/background.jpg");

    ui_state->elements.element_count = 0;
    cstrl_da_int_init(&ui_state->elements.ids, 1);
    cstrl_da_int_init(&ui_state->elements.screen_coords, 4);
    cstrl_da_float_init(&ui_state->elements.colors, 4);
    cstrl_da_string_init(&ui_state->elements.titles, 1);
    cstrl_da_int_init(&ui_state->elements.parent_index, 1);
    cstrl_da_int_init(&ui_state->elements.order_priority, 1);

    ui_state->element_cache.element_count = 0;
    cstrl_da_int_init(&ui_state->element_cache.ids, 1);
    cstrl_da_int_init(&ui_state->element_cache.screen_coords, 4);
    cstrl_da_float_init(&ui_state->element_cache.colors, 4);
    cstrl_da_string_init(&ui_state->element_cache.titles, 1);
    cstrl_da_int_init(&ui_state->element_cache.parent_index, 1);
    cstrl_da_int_init(&ui_state->element_cache.order_priority, 1);

    cstrl_da_int_init(&ui_state->element_render_order, 1);

    ui_state->dragged_element_id = -1;

    ui_state->mouse_x = -1;
    ui_state->mouse_y = -1;
    ui_state->prev_mouse_x = -1;
    ui_state->prev_mouse_y = -1;

    ui_state->left_mouse_button_down = false;
    ui_state->left_mouse_button_processed = false;
    ui_state->left_mouse_button_pressed_x = -1;
    ui_state->left_mouse_button_pressed_y = -1;

    ui_state->hot_item = -1;
    ui_state->active_item = -1;

    ui_state->key_pressed = CSTRL_KEY_NONE;
    ui_state->key_press_processed = false;

    cstrl_da_int_init(&ui_state->parent_stack, 1);

    return context;
}

CSTRL_API void cstrl_ui_begin(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    ui_state->elements.element_count = 0;
    cstrl_da_int_clear(&ui_state->elements.ids);
    cstrl_da_int_clear(&ui_state->elements.screen_coords);
    cstrl_da_float_clear(&ui_state->elements.colors);
    cstrl_da_string_clear(&ui_state->elements.titles);
    cstrl_da_int_clear(&ui_state->elements.parent_index);
    cstrl_da_int_clear(&ui_state->elements.order_priority);
    cstrl_da_int_clear(&ui_state->element_render_order);
    cstrl_da_int_clear(&ui_state->parent_stack);

    ui_state->hot_item = -1;
    ui_state->prev_mouse_x = ui_state->mouse_x;
    ui_state->prev_mouse_y = ui_state->mouse_y;
    cstrl_platform_get_cursor_position(ui_state->platform_state, &ui_state->mouse_x, &ui_state->mouse_y);
    bool previous_left_mouse_button_state = ui_state->left_mouse_button_down;
    ui_state->left_mouse_button_down =
        cstrl_platform_is_mouse_button_down(ui_state->platform_state, CSTRL_MOUSE_BUTTON_LEFT);
    if (!ui_state->left_mouse_button_down)
    {
        ui_state->left_mouse_button_pressed_x = -1;
        ui_state->left_mouse_button_pressed_y = -1;
        ui_state->dragged_element_id = -1;
    }
    else if (!previous_left_mouse_button_state)
    {
        ui_state->active_item = -1;
        ui_state->left_mouse_button_pressed_x = ui_state->mouse_x;
        ui_state->left_mouse_button_pressed_y = ui_state->mouse_y;
    }
    ui_state->key_pressed = cstrl_platform_get_most_recent_key_pressed(ui_state->platform_state);
    if (ui_state->key_pressed == CSTRL_KEY_NONE)
    {
        ui_state->key_press_processed = false;
    }
}

CSTRL_API void cstrl_ui_end(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    bool depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    cstrl_da_int_clear(&ui_state->element_cache.ids);
    cstrl_da_int_clear(&ui_state->element_cache.screen_coords);
    cstrl_da_float_clear(&ui_state->element_cache.colors);
    cstrl_da_string_clear(&ui_state->element_cache.titles);
    cstrl_da_int_clear(&ui_state->element_cache.parent_index);

    ui_state->key_pressed = CSTRL_KEY_NONE;

    ui_state->element_cache.element_count = ui_state->elements.element_count;

    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        cstrl_da_int_push_back(&ui_state->element_cache.ids, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        string s;
        cstrl_string_init(&s, 7);
        cstrl_string_push_back(&s, "Nothing", 7);
        cstrl_da_string_push_back(&ui_state->element_cache.titles, s);
        cstrl_string_free(&s);
        cstrl_da_int_push_back(&ui_state->element_cache.parent_index, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.order_priority, 0);
    }
    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        int index = ui_state->element_render_order.array[i];

        int x0 = ui_state->elements.screen_coords.array[index * 4];
        int y0 = ui_state->elements.screen_coords.array[index * 4 + 1];
        int x1 = ui_state->elements.screen_coords.array[index * 4 + 2];
        int y1 = ui_state->elements.screen_coords.array[index * 4 + 3];

        cstrl_da_float_push_back(&ui_state->positions, (float)x0);
        cstrl_da_float_push_back(&ui_state->positions, (float)y1);
        cstrl_da_float_push_back(&ui_state->positions, (float)x1);
        cstrl_da_float_push_back(&ui_state->positions, (float)y0);
        cstrl_da_float_push_back(&ui_state->positions, (float)x0);
        cstrl_da_float_push_back(&ui_state->positions, (float)y0);
        cstrl_da_float_push_back(&ui_state->positions, (float)x0);
        cstrl_da_float_push_back(&ui_state->positions, (float)y1);
        cstrl_da_float_push_back(&ui_state->positions, (float)x1);
        cstrl_da_float_push_back(&ui_state->positions, (float)y0);
        cstrl_da_float_push_back(&ui_state->positions, (float)x1);
        cstrl_da_float_push_back(&ui_state->positions, (float)y1);

        cstrl_da_float_push_back(&ui_state->uvs, 0.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 1.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 1.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 0.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 0.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 0.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 0.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 1.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 1.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 0.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 1.0f);
        cstrl_da_float_push_back(&ui_state->uvs, 1.0f);

        float r = ui_state->elements.colors.array[index * 4];
        float g = ui_state->elements.colors.array[index * 4 + 1];
        float b = ui_state->elements.colors.array[index * 4 + 2];
        float a = ui_state->elements.colors.array[index * 4 + 3];
        for (int j = 0; j < 6; j++)
        {
            cstrl_da_float_push_back(&ui_state->colors, r);
            cstrl_da_float_push_back(&ui_state->colors, g);
            cstrl_da_float_push_back(&ui_state->colors, b);
            cstrl_da_float_push_back(&ui_state->colors, a);
        }
        cstrl_renderer_modify_render_attributes(ui_state->render_data, ui_state->positions.array, ui_state->uvs.array,
                                                ui_state->colors.array, ui_state->positions.size / 2);
        cstrl_texture_bind(ui_state->texture);
        cstrl_use_shader(ui_state->shader);
        cstrl_renderer_draw(ui_state->render_data);

        cstrl_da_float_clear(&ui_state->positions);
        cstrl_da_float_clear(&ui_state->uvs);
        cstrl_da_float_clear(&ui_state->colors);

        ui_state->element_cache.ids.array[index] = ui_state->elements.ids.array[index];
        ui_state->element_cache.screen_coords.array[index * 4] = x0;
        ui_state->element_cache.screen_coords.array[index * 4 + 1] = y0;
        ui_state->element_cache.screen_coords.array[index * 4 + 2] = x1;
        ui_state->element_cache.screen_coords.array[index * 4 + 3] = y1;
        ui_state->element_cache.colors.array[index * 4] = r;
        ui_state->element_cache.colors.array[index * 4 + 1] = g;
        ui_state->element_cache.colors.array[index * 4 + 2] = b;
        ui_state->element_cache.colors.array[index * 4 + 3] = a;
        cstrl_string_free(&ui_state->element_cache.titles.array[index]);
        string cached_s;
        cstrl_string_init(&cached_s, ui_state->elements.titles.array[index].size);
        cstrl_string_push_back(&cached_s, ui_state->elements.titles.array[index].array,
                               ui_state->elements.titles.array[index].size);
        ui_state->element_cache.titles.array[index] = cached_s;
        ui_state->element_cache.parent_index.array[index] = ui_state->elements.parent_index.array[index];
        ui_state->element_cache.order_priority.array[index] = ui_state->elements.order_priority.array[index];

        if (ui_state->elements.titles.array[index].size == 0)
        {
            continue;
        }

        int h = y1 - y0;
        float font_h = h >= 30 ? 30.0f : (float)h;
        float scale = (font_h / 2.0f) / (float)FONT_SIZE;
        float position[2] = {(float)x0, (float)y0 + font_h / 1.5f};
        // if (alignment == CSTRL_UI_TEXT_ALIGN_CENTER)
        // {
        //     position[0] = (float)x + (float)w / 2.0f - cstrl_ui_text_width(context, text, scale) / 2.0f;
        // }
        // else if (alignment == CSTRL_UI_TEXT_ALIGN_RIGHT)
        // {
        //     position[0] = (float)x + (float)w - cstrl_ui_text_width(context, text, scale);
        // }

        string s = ui_state->elements.titles.array[index];

        for (int c = 0; c < s.size; c++)
        {
            stbtt_packedchar packedchar = ui_state->char_data[s.array[c]];

            // log_debug("%c", s.array[c]);

            float u0 = (float)packedchar.x0 / 512.0f;
            float v0 = (float)packedchar.y0 / 512.0f;
            float u1 = (float)packedchar.x1 / 512.0f;
            float v1 = (float)packedchar.y1 / 512.0f;

            float size[2] = {(packedchar.xoff2 - packedchar.xoff) * scale,
                             (packedchar.yoff2 - packedchar.yoff) * scale};

            float render_position[2] = {position[0] + packedchar.xoff * scale,
                                        position[1] - (packedchar.yoff2 - packedchar.yoff) * scale +
                                            packedchar.yoff2 * scale};

            cstrl_da_float_push_back(&ui_state->font_positions, render_position[0]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[1] + size[1]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[0] + size[0]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[1]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[0]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[1]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[0]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[1] + size[1]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[0] + size[0]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[1]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[0] + size[0]);
            cstrl_da_float_push_back(&ui_state->font_positions, render_position[1] + size[1]);

            cstrl_da_float_push_back(&ui_state->font_uvs, u0);
            cstrl_da_float_push_back(&ui_state->font_uvs, v1);
            cstrl_da_float_push_back(&ui_state->font_uvs, u1);
            cstrl_da_float_push_back(&ui_state->font_uvs, v0);
            cstrl_da_float_push_back(&ui_state->font_uvs, u0);
            cstrl_da_float_push_back(&ui_state->font_uvs, v0);
            cstrl_da_float_push_back(&ui_state->font_uvs, u0);
            cstrl_da_float_push_back(&ui_state->font_uvs, v1);
            cstrl_da_float_push_back(&ui_state->font_uvs, u1);
            cstrl_da_float_push_back(&ui_state->font_uvs, v0);
            cstrl_da_float_push_back(&ui_state->font_uvs, u1);
            cstrl_da_float_push_back(&ui_state->font_uvs, v1);

            for (int j = 0; j < 6; j++)
            {
                cstrl_da_float_push_back(&ui_state->font_colors, 0.0f);
                cstrl_da_float_push_back(&ui_state->font_colors, 0.0f);
                cstrl_da_float_push_back(&ui_state->font_colors, 0.0f);
                cstrl_da_float_push_back(&ui_state->font_colors, 1.0f);
            }

            position[0] += packedchar.xadvance * scale;
        }
        cstrl_renderer_modify_render_attributes(ui_state->font_render_data, ui_state->font_positions.array,
                                                ui_state->font_uvs.array, ui_state->font_colors.array,
                                                ui_state->font_positions.size / 2);
        cstrl_texture_bind(ui_state->font_texture);
        cstrl_use_shader(ui_state->font_shader);
        cstrl_renderer_draw(ui_state->font_render_data);

        cstrl_da_float_clear(&ui_state->font_positions);
        cstrl_da_float_clear(&ui_state->font_uvs);
        cstrl_da_float_clear(&ui_state->font_colors);
    }

    if (ui_state->left_mouse_button_down)
    {
        ui_state->left_mouse_button_processed = true;
    }
    else
    {
        ui_state->left_mouse_button_processed = false;
    }
    if (depth_test_enabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
}

CSTRL_API void cstrl_ui_shutdown(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    cstrl_da_int_free(&ui_state->elements.ids);
    cstrl_da_int_free(&ui_state->elements.screen_coords);
    cstrl_da_float_free(&ui_state->elements.colors);
    cstrl_da_int_free(&ui_state->elements.parent_index);
    cstrl_da_string_free(&ui_state->elements.titles);
    cstrl_da_int_free(&ui_state->elements.order_priority);
    cstrl_da_int_free(&ui_state->element_cache.ids);
    cstrl_da_int_free(&ui_state->element_cache.screen_coords);
    cstrl_da_float_free(&ui_state->element_cache.colors);
    cstrl_da_int_free(&ui_state->element_cache.parent_index);
    cstrl_da_string_free(&ui_state->element_cache.titles);
    cstrl_da_int_free(&ui_state->element_cache.order_priority);
    cstrl_da_int_free(&ui_state->element_render_order);

    cstrl_renderer_free_render_data(ui_state->render_data);
    cstrl_da_float_free(&ui_state->positions);
    cstrl_da_float_free(&ui_state->uvs);
    cstrl_da_float_free(&ui_state->colors);
    cstrl_renderer_free_render_data(ui_state->font_render_data);
    cstrl_da_float_free(&ui_state->font_positions);
    cstrl_da_float_free(&ui_state->font_uvs);
    cstrl_da_float_free(&ui_state->font_colors);

    free(ui_state->char_data);
    cstrl_da_int_free(&ui_state->parent_stack);
    free(ui_state);
    free(context);
}

CSTRL_API bool cstrl_ui_region_hit(int test_x, int test_y, int object_x, int object_y, int object_width,
                                   int object_height)
{
    if (test_x < object_x || test_y < object_y || test_x > object_x + object_width || test_y > object_y + object_height)
    {
        return false;
    }
    return true;
}

CSTRL_API float cstrl_ui_text_width(cstrl_ui_context *context, const char *text, float scale)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    int width = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        stbtt_packedchar c = ui_state->char_data[text[i]];
        width += c.xadvance;
    }
    return (float)width * scale;
}

CSTRL_API bool cstrl_ui_container_begin(cstrl_ui_context *context, const char *title, int title_length, int x, int y,
                                        int w, int h, int id, bool is_static, int order_priority)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    if (ui_state->parent_stack.size != 0)
    {
        log_error("CSTRL UI: Cannot have a container within a container");
        return false;
    }

    int original_w = w;
    int original_h = h;
    int original_order_priority = order_priority;
    if (ui_state->element_cache.element_count > 0)
    {
        x = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4];
        y = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4 + 1];
        w = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4 + 2] - x;
        h = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4 + 3] - y;
        order_priority = ui_state->element_cache.order_priority.array[ui_state->elements.element_count];
    }

    if (!is_static && cstrl_ui_region_hit(ui_state->mouse_x, ui_state->mouse_y, x, y, 30, 30) &&
        ui_state->left_mouse_button_down && !ui_state->left_mouse_button_processed)
    {
        if (w == original_w && h == original_h)
        {
            w = 50;
            h = 30;
        }
        else
        {
            w = original_w;
            h = original_h;
        }
    }
    if (!is_static && ui_state->dragged_element_id < 0 &&
        cstrl_ui_region_hit(ui_state->mouse_x, ui_state->mouse_y, x, y, w, h) && ui_state->left_mouse_button_down)
    {
        ui_state->dragged_element_id = id;
        ui_state->active_item = id;
    }
    if (!is_static && ui_state->dragged_element_id == id && ui_state->left_mouse_button_down)
    {
        // ui_state->dragged_element_id = id;
        ui_state->active_item = id;
        x -= ui_state->prev_mouse_x - ui_state->mouse_x;
        y -= ui_state->prev_mouse_y - ui_state->mouse_y;
    }

    if (ui_state->active_item == id)
    {
        order_priority = -1;
    }
    else
    {
        order_priority = original_order_priority;
    }
    if (x < 0)
    {
        x = 0;
    }
    else if (x > 800 - w)
    {
        x = 800 - w;
    }
    if (y < 0)
    {
        y = 0;
    }
    else if (y > 600 - h)
    {
        y = 600 - h;
    }

    cstrl_da_int_push_back(&ui_state->elements.order_priority, order_priority);
    bool inserted = false;
    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        if (order_priority >= ui_state->elements.order_priority.array[i])
        {
            cstrl_da_int_insert(&ui_state->element_render_order, ui_state->elements.element_count, i);
            inserted = true;
            break;
        }
    }
    if (!inserted)
    {
        cstrl_da_int_push_back(&ui_state->element_render_order, ui_state->elements.element_count);
    }

    cstrl_da_int_push_back(&ui_state->parent_stack, ui_state->elements.element_count);
    cstrl_da_int_push_back(&ui_state->elements.ids, id);

    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x + w);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y + h);

    // TODO: get from config somewhere
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.2f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.4f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.7f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.9f);

    string s;
    cstrl_string_init(&s, title_length);
    cstrl_string_push_back(&s, title, title_length);
    cstrl_da_string_push_back(&ui_state->elements.titles, s);
    cstrl_string_free(&s);

    cstrl_da_int_push_back(&ui_state->elements.parent_index, -1);

    ui_state->elements.element_count++;

    return true;
}

CSTRL_API void cstrl_ui_container_end(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    cstrl_da_int_pop_back(&ui_state->parent_stack);
}

CSTRL_API bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w,
                               int h, int id)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    cstrl_da_int_push_back(&ui_state->elements.ids, id);

    int parent_index = ui_state->parent_stack.array[ui_state->parent_stack.size - 1];
    x += ui_state->elements.screen_coords.array[parent_index * 4];
    y += ui_state->elements.screen_coords.array[parent_index * 4 + 1];
    int parent_w = ui_state->elements.screen_coords.array[parent_index * 4 + 2] -
                   ui_state->elements.screen_coords.array[parent_index * 4];
    int parent_h = ui_state->elements.screen_coords.array[parent_index * 4 + 3] -
                   ui_state->elements.screen_coords.array[parent_index * 4 + 1];

    if (w > parent_w)
    {
        w = parent_w;
    }
    if (h > parent_h)
    {
        h = parent_h;
    }

    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x + w);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y + h);

    string s;
    cstrl_string_init(&s, title_length);
    cstrl_string_push_back(&s, title, title_length);
    cstrl_da_string_push_back(&ui_state->elements.titles, s);
    cstrl_string_free(&s);

    cstrl_da_int_push_back(&ui_state->elements.parent_index, parent_index);

    int parent_priority = ui_state->elements.order_priority.array[parent_index];
    cstrl_da_int_push_back(&ui_state->elements.order_priority, parent_priority);
    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        if (ui_state->element_render_order.array[i] == parent_index)
        {
            cstrl_da_int_insert(&ui_state->element_render_order, ui_state->elements.element_count, i + 1);
            break;
        }
    }

    ui_state->elements.element_count++;

    if (cstrl_ui_region_hit(ui_state->mouse_x, ui_state->mouse_y, x, y, w, h))
    {
        ui_state->hot_item = id;
        if (ui_state->left_mouse_button_down &&
            (ui_state->active_item == -1 || ui_state->active_item == ui_state->elements.ids.array[parent_index]) &&
            cstrl_ui_region_hit(ui_state->left_mouse_button_pressed_x, ui_state->left_mouse_button_pressed_y, x, y, w,
                                h))
        {
            ui_state->active_item = ui_state->hot_item;
            // Pressed state
            cstrl_da_float_push_back(&ui_state->elements.colors, 0.2f);
            cstrl_da_float_push_back(&ui_state->elements.colors, 0.4f);
            cstrl_da_float_push_back(&ui_state->elements.colors, 0.6f);
            cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);
            return true;
        }
        // Hovered state
        cstrl_da_float_push_back(&ui_state->elements.colors, 0.4f);
        cstrl_da_float_push_back(&ui_state->elements.colors, 0.7f);
        cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);
        cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);
        return false;
    }
    // Default state
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.3f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.5f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.9f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);

    return false;
}

CSTRL_API bool cstrl_ui_text_field(cstrl_ui_context *context, const char *placeholder, int placeholder_length, int x,
                                   int y, int w, int h, int id, char *buffer, size_t buffer_size)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    cstrl_da_int_push_back(&ui_state->elements.ids, id);

    int parent_index = ui_state->parent_stack.array[ui_state->parent_stack.size - 1];
    x += ui_state->elements.screen_coords.array[parent_index * 4];
    y += ui_state->elements.screen_coords.array[parent_index * 4 + 1];
    int parent_w = ui_state->elements.screen_coords.array[parent_index * 4 + 2] -
                   ui_state->elements.screen_coords.array[parent_index * 4];
    int parent_h = ui_state->elements.screen_coords.array[parent_index * 4 + 3] -
                   ui_state->elements.screen_coords.array[parent_index * 4 + 1];

    if (w > parent_w)
    {
        w = parent_w;
    }
    if (h > parent_h)
    {
        h = parent_h;
    }

    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x + w);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y + h);

    // TODO: fix this monstrosity
    string s;
    cstrl_string_init(&s, placeholder_length);
    if (ui_state->element_cache.element_count > 0 && ui_state->active_item == id)
    {
        if (ui_state->element_cache.titles.array[ui_state->elements.element_count].size > 0)
        {
            cstrl_string_push_back(&s, ui_state->element_cache.titles.array[ui_state->elements.element_count].array,
                                   ui_state->element_cache.titles.array[ui_state->elements.element_count].size);
        }
        if (!ui_state->key_press_processed)
        {
            if (ui_state->key_pressed >= CSTRL_KEY_0 && ui_state->key_pressed <= CSTRL_KEY_9)
            {
                char key_char[1];
                key_char[0] = ui_state->key_pressed + 18;
                cstrl_string_push_back(&s, key_char, 1);
                ui_state->key_press_processed = true;
            }
            else if (ui_state->key_pressed == CSTRL_KEY_PERIOD)
            {
                char key_char[1];
                key_char[0] = '.';
                cstrl_string_push_back(&s, key_char, 1);
                ui_state->key_press_processed = true;
            }
            else if (ui_state->key_pressed == CSTRL_KEY_BACKSPACE)
            {
                if (s.size > 0)
                {
                    cstrl_string_pop_back(&s);
                }
                ui_state->key_press_processed = true;
            }
        }
    }
    else if (ui_state->element_cache.element_count <= 0)
    {
        cstrl_string_push_back(&s, placeholder, placeholder_length);
    }
    else
    {
        if (ui_state->element_cache.titles.array[ui_state->elements.element_count].size > 0)
        {
            cstrl_string_push_back(&s, ui_state->element_cache.titles.array[ui_state->elements.element_count].array,
                                   ui_state->element_cache.titles.array[ui_state->elements.element_count].size);
        }
    }

    cstrl_da_string_push_back(&ui_state->elements.titles, s);
    cstrl_string_free(&s);

    cstrl_da_int_push_back(&ui_state->elements.parent_index, parent_index);

    int parent_priority = ui_state->elements.order_priority.array[parent_index];
    cstrl_da_int_push_back(&ui_state->elements.order_priority, parent_priority);
    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        if (ui_state->element_render_order.array[i] == parent_index)
        {
            cstrl_da_int_insert(&ui_state->element_render_order, ui_state->elements.element_count, i + 1);
            break;
        }
    }

    cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 1.0f);

    size_t size = ui_state->elements.titles.array[ui_state->elements.element_count].size;
    size = size > buffer_size ? buffer_size : size;
    memcpy(buffer, ui_state->elements.titles.array[ui_state->elements.element_count].array, size);
    buffer[buffer_size - 1] = '\0';
    ui_state->elements.element_count++;

    if (cstrl_ui_region_hit(ui_state->mouse_x, ui_state->mouse_y, x, y, w, h))
    {
        ui_state->hot_item = id;
        if (ui_state->left_mouse_button_down &&
            (ui_state->active_item == -1 || ui_state->active_item == ui_state->elements.ids.array[parent_index]) &&
            cstrl_ui_region_hit(ui_state->left_mouse_button_pressed_x, ui_state->left_mouse_button_pressed_y, x, y, w,
                                h))
        {
            ui_state->active_item = ui_state->hot_item;
            cstrl_string_clear(&ui_state->elements.titles.array[ui_state->elements.element_count - 1]);
            return true;
        }
    }
    return false;
}
