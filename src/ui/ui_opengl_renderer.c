#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_util.h"
#include "ui_internal.h"
#include <stdio.h>
#include <stdlib.h>

#define FONT_SIZE 36

typedef struct cstrl_ui_internal_render_state
{
    cstrl_render_data *render_data;
    cstrl_shader shader;
    cstrl_texture texture;
    cstrl_render_data *font_render_data;
    cstrl_shader font_shader;
    cstrl_texture font_texture;
    stbtt_packedchar *char_data;

    mat4 projection;
    da_float positions;
    da_float uvs;
    da_float colors;
    da_float font_positions;
    da_float font_uvs;
    da_float font_colors;
} cstrl_ui_internal_render_state;

void *cstrl_ui_renderer_init(cstrl_platform_state *platform_state)
{
    cstrl_ui_internal_render_state *render_state = malloc(sizeof(cstrl_ui_internal_render_state));

    render_state->render_data = cstrl_renderer_create_render_data();
    render_state->font_render_data = cstrl_renderer_create_render_data();

    const char *vertex_shader_source = "                        \
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
    const char *fragment_shader_source = "                      \
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
    const char *font_vertex_shader_source = "                   \
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
    const char *font_fragment_shader_source = "                 \
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
    render_state->shader = cstrl_load_shaders_from_source(vertex_shader_source, fragment_shader_source);
    render_state->font_shader = cstrl_load_shaders_from_source(font_vertex_shader_source, font_fragment_shader_source);

    float positions[12] = {0.0f};
    float uvs[] = {0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    float colors[24] = {1.0f};
    cstrl_renderer_add_positions(render_state->render_data, positions, 2, 6);
    cstrl_renderer_add_uvs(render_state->render_data, uvs);
    cstrl_renderer_add_colors(render_state->render_data, colors);
    cstrl_renderer_add_positions(render_state->font_render_data, positions, 2, 6);
    cstrl_renderer_add_uvs(render_state->font_render_data, uvs);
    cstrl_renderer_add_colors(render_state->font_render_data, colors);

    int width, height;
    cstrl_platform_get_window_size(platform_state, &width, &height);
    render_state->projection = cstrl_mat4_ortho(0.0f, (float)width, (float)height, 0.0f, 0.1f, 100.0f);
    cstrl_set_uniform_mat4(render_state->shader.program, "projection", render_state->projection);
    cstrl_set_uniform_mat4(render_state->font_shader.program, "projection", render_state->projection);

    cstrl_da_float_init(&render_state->positions, 12);
    cstrl_da_float_init(&render_state->uvs, 12);
    cstrl_da_float_init(&render_state->colors, 24);
    cstrl_da_float_init(&render_state->font_positions, 12);
    cstrl_da_float_init(&render_state->font_uvs, 12);
    cstrl_da_float_init(&render_state->font_colors, 24);

    unsigned char texture[] = {0xff, 0xff, 0xff, 0xff};
    render_state->texture = cstrl_texture_generate_from_bitmap(texture, 1, 1, CSTRL_RGBA, CSTRL_RGBA);

    // The real mvp https://sinf.org/opengl-text-using-stb_truetype/
    // FILE *font_file = fopen("resources/fonts/SUSE/static/SUSE-Regular.ttf", "rb");
    FILE *font_file = fopen("resources/fonts/Jersey_20/Jersey20-Regular.ttf", "rb");
    fseek(font_file, 0, SEEK_END);
    long size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    unsigned char *font_buffer = malloc(size);

    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    stbtt_pack_context pack_context;
    render_state->char_data = calloc(126, sizeof(stbtt_packedchar));
    unsigned char *pixels = calloc(512 * 512, sizeof(char));
    stbtt_PackBegin(&pack_context, pixels, 512, 512, 512, 1, NULL);
    stbtt_PackFontRange(&pack_context, font_buffer, 0, FONT_SIZE, 0, 125, render_state->char_data);
    stbtt_PackEnd(&pack_context);

    render_state->font_texture = cstrl_texture_generate_from_bitmap(pixels, 512, 512, CSTRL_RED, CSTRL_RED);

    free(pixels);
    free(font_buffer);

    return render_state;
}

stbtt_packedchar *cstrl_ui_renderer_get_char_data(void *internal_render_state)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    return render_state->char_data;
}

void cstrl_ui_renderer_clear_data(void *internal_render_state)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_da_float_clear(&render_state->positions);
    cstrl_da_float_clear(&render_state->uvs);
    cstrl_da_float_clear(&render_state->colors);
    cstrl_da_float_clear(&render_state->font_positions);
    cstrl_da_float_clear(&render_state->font_uvs);
    cstrl_da_float_clear(&render_state->font_colors);
}

void cstrl_ui_renderer_add_rect_position(void *internal_render_state, int x0, int y0, int x1, int y1)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_da_float_push_back(&render_state->positions, (float)x0);
    cstrl_da_float_push_back(&render_state->positions, (float)y1);
    cstrl_da_float_push_back(&render_state->positions, (float)x1);
    cstrl_da_float_push_back(&render_state->positions, (float)y0);
    cstrl_da_float_push_back(&render_state->positions, (float)x0);
    cstrl_da_float_push_back(&render_state->positions, (float)y0);
    cstrl_da_float_push_back(&render_state->positions, (float)x0);
    cstrl_da_float_push_back(&render_state->positions, (float)y1);
    cstrl_da_float_push_back(&render_state->positions, (float)x1);
    cstrl_da_float_push_back(&render_state->positions, (float)y0);
    cstrl_da_float_push_back(&render_state->positions, (float)x1);
    cstrl_da_float_push_back(&render_state->positions, (float)y1);
}

void cstrl_ui_renderer_add_rect_uv(void *internal_render_state, float u0, float v0, float u1, float v1)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_da_float_push_back(&render_state->uvs, u0);
    cstrl_da_float_push_back(&render_state->uvs, v1);
    cstrl_da_float_push_back(&render_state->uvs, u1);
    cstrl_da_float_push_back(&render_state->uvs, v0);
    cstrl_da_float_push_back(&render_state->uvs, u0);
    cstrl_da_float_push_back(&render_state->uvs, v0);
    cstrl_da_float_push_back(&render_state->uvs, u0);
    cstrl_da_float_push_back(&render_state->uvs, u1);
    cstrl_da_float_push_back(&render_state->uvs, v1);
    cstrl_da_float_push_back(&render_state->uvs, v0);
    cstrl_da_float_push_back(&render_state->uvs, u1);
    cstrl_da_float_push_back(&render_state->uvs, v1);
}

void cstrl_ui_renderer_add_rect_color(void *internal_render_state, float r, float g, float b, float a)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    for (int j = 0; j < 6; j++)
    {
        cstrl_da_float_push_back(&render_state->colors, r);
        cstrl_da_float_push_back(&render_state->colors, g);
        cstrl_da_float_push_back(&render_state->colors, b);
        cstrl_da_float_push_back(&render_state->colors, a);
    }
}

void cstrl_ui_renderer_draw_rects(void *internal_render_state)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_use_shader(render_state->shader);
    cstrl_texture_bind(render_state->texture);
    cstrl_renderer_modify_render_attributes(render_state->render_data, render_state->positions.array,
                                            render_state->uvs.array, render_state->colors.array,
                                            render_state->positions.size / 2);
    cstrl_renderer_draw(render_state->render_data);
}

void cstrl_ui_renderer_add_font(void *internal_render_state, char *text, unsigned int start, unsigned int end,
                                int start_x, int start_y, float r, float g, float b, float a)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    int next_x = start_x;
    for (int i = start; i < end; i++)
    {
        stbtt_packedchar packedchar = render_state->char_data[text[i]];
        float x0 = (float)next_x + packedchar.xoff;
        float y0 = FONT_SIZE / 2.0f + (float)start_y + packedchar.yoff2;
        float x1 = (float)next_x + (packedchar.xoff2 - packedchar.xoff);
        float y1 = FONT_SIZE / 2.0f + (float)start_y + packedchar.yoff;

        cstrl_da_float_push_back(&render_state->font_positions, x0);
        cstrl_da_float_push_back(&render_state->font_positions, y1);
        cstrl_da_float_push_back(&render_state->font_positions, x1);
        cstrl_da_float_push_back(&render_state->font_positions, y0);
        cstrl_da_float_push_back(&render_state->font_positions, x0);
        cstrl_da_float_push_back(&render_state->font_positions, y0);
        cstrl_da_float_push_back(&render_state->font_positions, x0);
        cstrl_da_float_push_back(&render_state->font_positions, y1);
        cstrl_da_float_push_back(&render_state->font_positions, x1);
        cstrl_da_float_push_back(&render_state->font_positions, y0);
        cstrl_da_float_push_back(&render_state->font_positions, x1);
        cstrl_da_float_push_back(&render_state->font_positions, y1);

        float u0 = (float)packedchar.x0 / 512.0f;
        float v0 = (float)packedchar.y1 / 512.0f;
        float u1 = (float)packedchar.x1 / 512.0f;
        float v1 = (float)packedchar.y0 / 512.0f;

        cstrl_da_float_push_back(&render_state->font_uvs, u0);
        cstrl_da_float_push_back(&render_state->font_uvs, v1);
        cstrl_da_float_push_back(&render_state->font_uvs, u1);
        cstrl_da_float_push_back(&render_state->font_uvs, v0);
        cstrl_da_float_push_back(&render_state->font_uvs, u0);
        cstrl_da_float_push_back(&render_state->font_uvs, v0);
        cstrl_da_float_push_back(&render_state->font_uvs, u0);
        cstrl_da_float_push_back(&render_state->font_uvs, v1);
        cstrl_da_float_push_back(&render_state->font_uvs, u1);
        cstrl_da_float_push_back(&render_state->font_uvs, v0);
        cstrl_da_float_push_back(&render_state->font_uvs, u1);
        cstrl_da_float_push_back(&render_state->font_uvs, v1);

        for (int j = 0; j < 6; j++)
        {
            cstrl_da_float_push_back(&render_state->font_colors, r);
            cstrl_da_float_push_back(&render_state->font_colors, g);
            cstrl_da_float_push_back(&render_state->font_colors, b);
            cstrl_da_float_push_back(&render_state->font_colors, a);
        }

        next_x += packedchar.xadvance;
    }
}

void cstrl_ui_renderer_add_font_uv(void *internal_render_state, float u0, float v0, float u1, float v1)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_da_float_push_back(&render_state->font_uvs, u0);
    cstrl_da_float_push_back(&render_state->font_uvs, v1);
    cstrl_da_float_push_back(&render_state->font_uvs, u1);
    cstrl_da_float_push_back(&render_state->font_uvs, v0);
    cstrl_da_float_push_back(&render_state->font_uvs, u0);
    cstrl_da_float_push_back(&render_state->font_uvs, v0);
    cstrl_da_float_push_back(&render_state->font_uvs, u0);
    cstrl_da_float_push_back(&render_state->font_uvs, u1);
    cstrl_da_float_push_back(&render_state->font_uvs, v1);
    cstrl_da_float_push_back(&render_state->font_uvs, v0);
    cstrl_da_float_push_back(&render_state->font_uvs, u1);
    cstrl_da_float_push_back(&render_state->font_uvs, v1);
}

void cstrl_ui_renderer_add_font_color(void *internal_render_state, float r, float g, float b, float a)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    for (int j = 0; j < 6; j++)
    {
        cstrl_da_float_push_back(&render_state->colors, r);
        cstrl_da_float_push_back(&render_state->colors, g);
        cstrl_da_float_push_back(&render_state->colors, b);
        cstrl_da_float_push_back(&render_state->colors, a);
    }
}

void cstrl_ui_renderer_draw_font(void *internal_render_state)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_use_shader(render_state->font_shader);
    cstrl_texture_bind(render_state->font_texture);
    cstrl_renderer_modify_render_attributes(render_state->font_render_data, render_state->font_positions.array,
                                            render_state->font_uvs.array, render_state->font_colors.array,
                                            render_state->font_positions.size / 2);
    cstrl_renderer_draw(render_state->font_render_data);
}

void cstrl_ui_renderer_shutdown(void *internal_render_state)
{
    cstrl_ui_internal_render_state *render_state = (cstrl_ui_internal_render_state *)internal_render_state;
    cstrl_da_float_free(&render_state->positions);
    cstrl_da_float_free(&render_state->uvs);
    cstrl_da_float_free(&render_state->colors);
    cstrl_da_float_free(&render_state->font_positions);
    cstrl_da_float_free(&render_state->font_uvs);
    cstrl_da_float_free(&render_state->font_colors);

    cstrl_renderer_free_render_data(render_state->render_data);
    cstrl_renderer_free_render_data(render_state->font_render_data);
    free(render_state->char_data);
}
