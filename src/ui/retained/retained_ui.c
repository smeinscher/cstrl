#include "log.c/log.h"
#include <cstrl/cstrl_platform.h>
#include <cstrl/cstrl_renderer.h>
#include <cstrl/cstrl_retained_ui.h>
#include <stb/stb_image_write.h>
#include <stb/stb_truetype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_UI_ELEMENTS 128
#define MAX_TEXT_COUNT 2048

static struct manager_t
{
    cstrl_retained_ui_element_t elements[MAX_UI_ELEMENTS];
    int element_count;
    int text_count;
} g_manager = {0};

static cstrl_render_data *g_rect_render_data = NULL;
static cstrl_render_data *g_font_render_data = NULL;
static cstrl_shader g_rect_shader;
static cstrl_shader g_font_shader;
static cstrl_texture g_rect_texture;
static stbtt_packedchar *g_char_data[CSTRL_RETAINED_UI_MAX_FONT_SIZE] = {0};
static cstrl_texture_array g_font_texture_array;
static mat4 g_projection = {0};
static float *g_rect_buffer_positions = NULL;
static float *g_rect_buffer_uvs = NULL;
static float *g_rect_buffer_colors = NULL;
static float *g_font_buffer_positions = NULL;
static float *g_font_buffer_uvs = NULL;
static float *g_font_buffer_colors = NULL;
static float *g_font_buffer_layers = NULL;

static float g_font_size_array[] = {16, 20, 24, 28, 32, 36};

static bool g_mouse_over_ui = false;
static bool g_element_clicked = false;

static void generate_char_data(unsigned char *font_buffer, float font_size, unsigned char **bitmaps, int bitmap_index)
{
    stbtt_pack_context pack_context;
    g_char_data[bitmap_index] = calloc(126, sizeof(stbtt_packedchar));
    unsigned char *pixels = calloc(512 * 512, sizeof(char));
    stbtt_PackBegin(&pack_context, pixels, 512, 512, 512, 1, NULL);
    stbtt_PackFontRange(&pack_context, font_buffer, 0, font_size, 0, 125, g_char_data[bitmap_index]);
    stbtt_PackEnd(&pack_context);
    bitmaps[bitmap_index] = pixels;
}

CSTRL_API void cstrl_retained_ui_init(cstrl_platform_state *platform_state, const char *texture_path,
                                      const char *font_path)
{
    g_rect_render_data = cstrl_renderer_create_render_data();
    g_font_render_data = cstrl_renderer_create_render_data();

    float positions[MAX_UI_ELEMENTS * 12] = {0};
    float uvs[MAX_UI_ELEMENTS * 12] = {0};
    float colors[MAX_UI_ELEMENTS * 24] = {0};

    cstrl_renderer_add_positions(g_rect_render_data, positions, 2, 6 * MAX_UI_ELEMENTS);
    cstrl_renderer_add_uvs(g_rect_render_data, uvs);
    cstrl_renderer_add_colors(g_rect_render_data, colors);

    float font_positions[MAX_TEXT_COUNT * 12] = {0};
    float font_uvs[MAX_TEXT_COUNT * 12] = {0};
    float font_colors[MAX_TEXT_COUNT * 24] = {0};
    float font_layers[MAX_TEXT_COUNT * 6] = {0};

    cstrl_renderer_add_positions(g_font_render_data, font_positions, 2, 6 * MAX_TEXT_COUNT);
    cstrl_renderer_add_uvs(g_font_render_data, font_uvs);
    cstrl_renderer_add_colors(g_font_render_data, font_colors);
    cstrl_renderer_add_layers(g_font_render_data, font_layers);

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
        layout (location = 7) in float a_layer;                     \
                                                                    \
        out vec2 uv;                                                \
        out vec4 color;                                             \
        out float layer;                                            \
                                                                    \
        uniform mat4 projection;                                    \
                                                                    \
        void main()                                                 \
        {                                                           \
            gl_Position = projection * vec4(a_pos, -1.0, 1.0);      \
            uv = a_uv;                                              \
            color = a_color;                                        \
            layer = a_layer;                                        \
        }";
    const char *font_fragment_shader_source = "                     \
        #version 460 core\n                                         \
                                                                    \
        out vec4 frag_color;                                        \
                                                                    \
        in vec2 uv;                                                 \
        in vec4 color;                                              \
        in float layer;                                             \
                                                                    \
        uniform sampler2DArray font_textures;                       \
                                                                    \
        void main()                                                 \
        {                                                           \
            frag_color = vec4(color.rgb,                            \
                color.a * texture(font_textures, vec3(uv, layer)).r);\
        }";
    g_rect_shader = cstrl_load_shaders_from_source(vertex_shader_source, fragment_shader_source);
    g_font_shader = cstrl_load_shaders_from_source(font_vertex_shader_source, font_fragment_shader_source);
    g_rect_texture = cstrl_texture_generate_from_path(texture_path, CSTRL_TEXTURE_FILTER_LINEAR);

    g_rect_buffer_positions = cstrl_renderer_map_positions_range(g_rect_render_data, positions);
    g_rect_buffer_uvs = cstrl_renderer_map_uvs_range(g_rect_render_data, uvs);
    g_rect_buffer_colors = cstrl_renderer_map_colors_range(g_rect_render_data, colors);
    g_font_buffer_positions = cstrl_renderer_map_positions_range(g_font_render_data, font_positions);
    g_font_buffer_uvs = cstrl_renderer_map_uvs_range(g_font_render_data, font_uvs);
    g_font_buffer_colors = cstrl_renderer_map_colors_range(g_font_render_data, font_colors);
    g_font_buffer_layers = cstrl_renderer_map_layers_range(g_font_render_data, font_layers);

    FILE *font_file = fopen(font_path, "rb");
    fseek(font_file, 0, SEEK_END);
    long size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    unsigned char *font_buffer = malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);
    unsigned char *bitmaps[CSTRL_RETAINED_UI_MAX_FONT_SIZE];
    for (int i = 0; i < CSTRL_RETAINED_UI_MAX_FONT_SIZE; i++)
    {
        generate_char_data(font_buffer, g_font_size_array[i], bitmaps, i);
    }
    g_font_texture_array = cstrl_texture_array_generate_from_bitmaps(CSTRL_RETAINED_UI_MAX_FONT_SIZE, bitmaps, 512, 512,
                                                                     CSTRL_RED, CSTRL_RED);
    for (int i = 0; i < CSTRL_RETAINED_UI_MAX_FONT_SIZE; i++)
    {
        free(bitmaps[i]);
    }
    free(font_buffer);

    int width, height;
    cstrl_platform_get_viewport_size(platform_state, &width, &height);
    g_projection = cstrl_mat4_ortho(0.0f, (float)width, (float)height, 0.0f, 0.1f, 100.0f);
    cstrl_set_uniform_mat4(g_rect_shader.program, "projection", g_projection);
    cstrl_set_uniform_mat4(g_font_shader.program, "projection", g_projection);
}

CSTRL_API void cstrl_retained_ui_get_text_size(const char *text, float *width, float *height,
                                               cstrl_retained_ui_font_size_t font_size_index)
{
    *width = 0.0f;
    *height = 0.0f;
    for (int j = 0; j < strlen(text); j++)
    {
        *width += g_char_data[font_size_index][text[j]].xadvance;
        *height = fmaxf(g_char_data[font_size_index][text[j]].y1 - g_char_data[font_size_index][text[j]].y0, *height);
    }
}

CSTRL_API cstrl_retained_ui_element_t *cstrl_retained_ui_create(cstrl_retained_ui_type_t type, char *label,
                                                                cstrl_retained_ui_font_size_t label_font_size)
{
    if (g_manager.element_count >= MAX_UI_ELEMENTS)
    {
        log_error("Max element count reached");
        return NULL;
    }
    if (g_manager.text_count >= MAX_TEXT_COUNT)
    {
        log_error("Max text count reached");
        return NULL;
    }
    cstrl_retained_ui_element_t *element = &g_manager.elements[g_manager.element_count];
    memset(element, 0, sizeof(cstrl_retained_ui_element_t));
    element->_type = type;
    element->_font_size = label_font_size;
    element->texture_u0 = 0.0f;
    element->texture_v0 = 0.0f;
    element->texture_u1 = 1.0f;
    element->texture_v1 = 1.0f;
    element->_id = g_manager.element_count++;
    element->parent_id = -1;
    element->label = malloc((strlen(label) + 1) * sizeof(char));
    element->_label_max_length = strlen(label);
    strcpy(element->label, label);
    element->label_index = g_manager.text_count;
    g_manager.text_count += strlen(label);
    element->color = (cstrl_retained_ui_color_t){1.0f, 1.0f, 1.0f, 1.0f};
    element->hovered_color = (cstrl_retained_ui_color_t){1.0f, 1.0f, 1.0f, 1.0f};
    element->font_color = (cstrl_retained_ui_color_t){1.0f, 1.0f, 1.0f, 1.0f};
    element->font_hovered_color = (cstrl_retained_ui_color_t){1.0f, 1.0f, 1.0f, 1.0f};

    for (int i = element->label_index * 6; i < g_manager.text_count * 6; i++)
    {
        g_font_buffer_layers[i] = label_font_size;
    }

    return element;
}

CSTRL_API void cstrl_retained_ui_update(cstrl_platform_state *platform_state)
{
    int width, height;
    cstrl_platform_get_viewport_size(platform_state, &width, &height);
    g_projection = cstrl_mat4_ortho(0.0f, (float)width, (float)height, 0.0f, 0.1f, 100.0f);
    g_mouse_over_ui = false;
    bool mouse_button_down = cstrl_platform_is_mouse_button_down(platform_state, CSTRL_MOUSE_BUTTON_LEFT);
    if (!mouse_button_down)
    {
        g_element_clicked = false;
    }
    for (int i = 0; i < g_manager.element_count; i++)
    {
        cstrl_retained_ui_element_t *element = &g_manager.elements[i];

        if (!element->visible)
        {
            continue;
        }

        int mouse_x, mouse_y;
        cstrl_platform_get_cursor_position(platform_state, &mouse_x, &mouse_y);

        bool inside = mouse_x >= element->x && mouse_x <= element->x + element->width && mouse_y >= element->y &&
                      mouse_y <= element->y + element->height;

        element->hovered = inside;

        g_mouse_over_ui = inside || g_mouse_over_ui;

        if (element->_type == CSTRL_RETAINED_UI_BUTTON && inside && mouse_button_down)
        {
            if (!element->clicked && !g_element_clicked)
            {
                element->clicked = true;

                if (element->on_click)
                {
                    element->on_click(element);
                }
                g_element_clicked = true;
            }
        }
        else
        {
            element->clicked = false;
        }
    }
}

CSTRL_API void cstrl_retained_ui_render()
{
    for (int i = 0; i < g_manager.element_count; i++)
    {
        cstrl_retained_ui_element_t *element = &g_manager.elements[i];

        memset(g_rect_buffer_positions + i * 12, 0, 12 * sizeof(float));
        int idx = element->label_index;
        memset(g_font_buffer_positions + idx * 12, 0, element->_label_max_length * 12 * sizeof(float));
        if (!element->visible)
        {
            continue;
        }
        float x0 = element->x;
        float y0 = element->y;
        float x1 = element->x + element->width;
        float y1 = element->y + element->height;
        g_rect_buffer_positions[i * 12] = x0;
        g_rect_buffer_positions[i * 12 + 1] = y1;
        g_rect_buffer_positions[i * 12 + 2] = x1;
        g_rect_buffer_positions[i * 12 + 3] = y0;
        g_rect_buffer_positions[i * 12 + 4] = x0;
        g_rect_buffer_positions[i * 12 + 5] = y0;
        g_rect_buffer_positions[i * 12 + 6] = x0;
        g_rect_buffer_positions[i * 12 + 7] = y1;
        g_rect_buffer_positions[i * 12 + 8] = x1;
        g_rect_buffer_positions[i * 12 + 9] = y0;
        g_rect_buffer_positions[i * 12 + 10] = x1;
        g_rect_buffer_positions[i * 12 + 11] = y1;

        float u0 = element->texture_u0;
        float v0 = element->texture_v0;
        float u1 = element->texture_u1;
        float v1 = element->texture_v1;
        g_rect_buffer_uvs[i * 12] = u0;
        g_rect_buffer_uvs[i * 12 + 1] = v1;
        g_rect_buffer_uvs[i * 12 + 2] = u1;
        g_rect_buffer_uvs[i * 12 + 3] = v0;
        g_rect_buffer_uvs[i * 12 + 4] = u0;
        g_rect_buffer_uvs[i * 12 + 5] = v0;
        g_rect_buffer_uvs[i * 12 + 6] = u0;
        g_rect_buffer_uvs[i * 12 + 7] = v1;
        g_rect_buffer_uvs[i * 12 + 8] = u1;
        g_rect_buffer_uvs[i * 12 + 9] = v0;
        g_rect_buffer_uvs[i * 12 + 10] = u1;
        g_rect_buffer_uvs[i * 12 + 11] = v1;

        float next_x = x0 + element->label_x;
        float start_y = y0 + element->label_y;
        int font_size_index = element->_font_size;
        for (int j = 0; j < strlen(element->label); j++)
        {
            x0 = (float)next_x + g_char_data[font_size_index][element->label[j]].xoff;
            y0 = g_font_size_array[font_size_index] / 2.0f + (float)start_y +
                 g_char_data[font_size_index][element->label[j]].yoff2;
            x1 = (float)next_x + (g_char_data[font_size_index][element->label[j]].xoff2 -
                                  g_char_data[font_size_index][element->label[j]].xoff);
            y1 = g_font_size_array[font_size_index] / 2.0f + (float)start_y +
                 g_char_data[font_size_index][element->label[j]].yoff;
            next_x += g_char_data[font_size_index][element->label[j]].xadvance;
            g_font_buffer_positions[idx * 12 + j * 12] = x0;
            g_font_buffer_positions[idx * 12 + j * 12 + 1] = y1;
            g_font_buffer_positions[idx * 12 + j * 12 + 2] = x1;
            g_font_buffer_positions[idx * 12 + j * 12 + 3] = y0;
            g_font_buffer_positions[idx * 12 + j * 12 + 4] = x0;
            g_font_buffer_positions[idx * 12 + j * 12 + 5] = y0;
            g_font_buffer_positions[idx * 12 + j * 12 + 6] = x0;
            g_font_buffer_positions[idx * 12 + j * 12 + 7] = y1;
            g_font_buffer_positions[idx * 12 + j * 12 + 8] = x1;
            g_font_buffer_positions[idx * 12 + j * 12 + 9] = y0;
            g_font_buffer_positions[idx * 12 + j * 12 + 10] = x1;
            g_font_buffer_positions[idx * 12 + j * 12 + 11] = y1;
            u0 = (float)g_char_data[font_size_index][element->label[j]].x0 / 512.0f;
            v0 = (float)g_char_data[font_size_index][element->label[j]].y1 / 512.0f;
            u1 = (float)g_char_data[font_size_index][element->label[j]].x1 / 512.0f;
            v1 = (float)g_char_data[font_size_index][element->label[j]].y0 / 512.0f;
            g_font_buffer_uvs[idx * 12 + j * 12] = u0;
            g_font_buffer_uvs[idx * 12 + j * 12 + 1] = v1;
            g_font_buffer_uvs[idx * 12 + j * 12 + 2] = u1;
            g_font_buffer_uvs[idx * 12 + j * 12 + 3] = v0;
            g_font_buffer_uvs[idx * 12 + j * 12 + 4] = u0;
            g_font_buffer_uvs[idx * 12 + j * 12 + 5] = v0;
            g_font_buffer_uvs[idx * 12 + j * 12 + 6] = u0;
            g_font_buffer_uvs[idx * 12 + j * 12 + 7] = v1;
            g_font_buffer_uvs[idx * 12 + j * 12 + 8] = u1;
            g_font_buffer_uvs[idx * 12 + j * 12 + 9] = v0;
            g_font_buffer_uvs[idx * 12 + j * 12 + 10] = u1;
            g_font_buffer_uvs[idx * 12 + j * 12 + 11] = v1;
            if (element->hovered)
            {
                for (int k = 0; k < 6; k++)
                {
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4] = element->font_hovered_color.r;
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4 + 1] = element->font_hovered_color.g;
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4 + 2] = element->font_hovered_color.b;
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4 + 3] = element->font_hovered_color.a;
                }
            }
            else
            {

                for (int k = 0; k < 6; k++)
                {
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4] = element->font_color.r;
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4 + 1] = element->font_color.g;
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4 + 2] = element->font_color.b;
                    g_font_buffer_colors[idx * 24 + j * 24 + k * 4 + 3] = element->font_color.a;
                }
            }
        }

        if (element->hovered)
        {
            for (int j = 0; j < 6; j++)
            {
                g_rect_buffer_colors[i * 24 + j * 4] = element->hovered_color.r;
                g_rect_buffer_colors[i * 24 + j * 4 + 1] = element->hovered_color.g;
                g_rect_buffer_colors[i * 24 + j * 4 + 2] = element->hovered_color.b;
                g_rect_buffer_colors[i * 24 + j * 4 + 3] = element->hovered_color.a;
            }
        }
        else
        {
            for (int j = 0; j < 6; j++)
            {
                g_rect_buffer_colors[i * 24 + j * 4] = element->color.r;
                g_rect_buffer_colors[i * 24 + j * 4 + 1] = element->color.g;
                g_rect_buffer_colors[i * 24 + j * 4 + 2] = element->color.b;
                g_rect_buffer_colors[i * 24 + j * 4 + 3] = element->color.a;
            }
        }
    }
    cstrl_use_shader(g_rect_shader);
    cstrl_set_uniform_mat4(g_rect_shader.program, "projection", g_projection);
    cstrl_texture_bind(g_rect_texture);
    cstrl_renderer_draw(g_rect_render_data);

    cstrl_use_shader(g_font_shader);
    cstrl_set_uniform_mat4(g_font_shader.program, "projection", g_projection);
    cstrl_texture_array_bind(g_font_texture_array);
    cstrl_renderer_draw(g_font_render_data);
}

CSTRL_API cstrl_retained_ui_element_t *cstrl_retained_ui_get_element_by_id(int id)
{
    return &g_manager.elements[id];
}

CSTRL_API bool cstrl_retained_ui_is_mouse_over_ui()
{
    return g_mouse_over_ui;
}

CSTRL_API void cstrl_retained_ui_shutdown()
{
    for (int i = 0; i < g_manager.element_count; i++)
    {
        free(g_manager.elements[i].label);
        cstrl_da_int_free(&g_manager.elements[i].child_ids);
    }
    cstrl_renderer_unmap_positions_range(g_rect_render_data);
    cstrl_renderer_unmap_uvs_range(g_rect_render_data);
    cstrl_renderer_unmap_colors_range(g_rect_render_data);
    cstrl_renderer_unmap_positions_range(g_font_render_data);
    cstrl_renderer_unmap_uvs_range(g_font_render_data);
    cstrl_renderer_unmap_colors_range(g_font_render_data);
    cstrl_renderer_unmap_layers_range(g_font_render_data);
    cstrl_renderer_free_render_data(g_rect_render_data);
    g_rect_render_data = NULL;
    cstrl_renderer_free_render_data(g_font_render_data);
    g_font_render_data = NULL;
    for (int i = 0; i < CSTRL_RETAINED_UI_MAX_FONT_SIZE; i++)
    {
        free(g_char_data[i]);
        g_char_data[i] = NULL;
    }
    g_projection = (mat4){0};
    memset(g_manager.elements, 0, MAX_UI_ELEMENTS * sizeof(cstrl_retained_ui_element_t));
    g_manager.element_count = 0;
    g_manager.text_count = 0;
}
