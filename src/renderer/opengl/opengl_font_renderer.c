#include <cstrl/cstrl_renderer.h>
#include <stb/stb_truetype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: dynamic size
#define MAX_TEXT_COUNT 4096
#define TEXT_BLOCK_SIZE 32

typedef enum font_buffer_type
{
    FONT_BUFFER_TYPE_POSITIONS,
    FONT_BUFFER_TYPE_UVS,
    FONT_BUFFER_TYPE_COLORS,
    FONT_MAX_BUFFER_TYPE
} font_buffer_type;

typedef struct internal_data
{
    float *font_buffers[FONT_MAX_BUFFER_TYPE];
    cstrl_shader shader;
    cstrl_texture texture;
    cstrl_render_data *render_data;
    stbtt_packedchar *char_data;
    float font_size;
} internal_data;

const char *font_vertex_shader_source = "                           \
        #version 460 core\n                                         \
        layout (location = 0) in vec2 a_pos;                        \
        layout (location = 1) in vec2 a_uv;                         \
        layout (location = 2) in vec4 a_color;                      \
                                                                    \
        out vec2 uv;                                                \
        out vec4 color;                                             \
                                                                    \
        uniform mat4 projection;                                    \
        uniform mat4 view;                                          \
                                                                    \
        void main()                                                 \
        {                                                           \
            gl_Position = projection * view * vec4(a_pos, -1.0, 1.0);\
            uv = a_uv;                                              \
            color = a_color;                                        \
        }";
const char *font_fragment_shader_source = "                         \
        #version 460 core\n                                         \
                                                                    \
        out vec4 frag_color;                                        \
                                                                    \
        in vec2 uv;                                                 \
        in vec4 color;                                              \
                                                                    \
        uniform sampler2D font_texture;                             \
                                                                    \
        void main()                                                 \
        {                                                           \
            frag_color = vec4(color.rgb, color.a * texture(font_texture, uv).r);  \
        }";

// TODO: check malloc/calloc failure
CSTRL_API cstrl_font_data *cstrl_font_renderer_create_font_data(const char *font_path, float font_size)
{
    cstrl_font_data *data = malloc(sizeof(cstrl_font_data));
    data->internal_data = malloc(sizeof(internal_data));
    internal_data *internal_data = data->internal_data;
    internal_data->render_data = cstrl_renderer_create_render_data();

    internal_data->font_size = font_size;

    float positions[MAX_TEXT_COUNT * 12] = {0};
    float uvs[MAX_TEXT_COUNT * 12] = {0};
    float colors[MAX_TEXT_COUNT * 24] = {0};

    cstrl_renderer_add_positions(internal_data->render_data, positions, 2, MAX_TEXT_COUNT * 6, CSTRL_USAGE_DYNAMIC);
    cstrl_renderer_add_uvs(internal_data->render_data, uvs, CSTRL_USAGE_DYNAMIC);
    cstrl_renderer_add_colors(internal_data->render_data, colors, CSTRL_USAGE_DYNAMIC);

    internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS] =
        cstrl_renderer_map_positions_range(internal_data->render_data, positions);
    internal_data->font_buffers[FONT_BUFFER_TYPE_UVS] = cstrl_renderer_map_uvs_range(internal_data->render_data, uvs);
    internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS] =
        cstrl_renderer_map_colors_range(internal_data->render_data, colors);

    internal_data->shader = cstrl_load_shaders_from_source(font_vertex_shader_source, font_fragment_shader_source);

#ifdef CSTRL_PLATFORM_WINDOWS
    FILE *font_file;
    fopen_s(&font_file, font_path, "rb");
#else
    FILE *font_file = fopen(font_path, "rb");
#endif
    fseek(font_file, 0, SEEK_END);
    long size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    unsigned char *font_buffer = malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);

    stbtt_pack_context pack_context;
    internal_data->char_data = calloc(126, sizeof(stbtt_packedchar));
    unsigned char *pixels = calloc(512 * 512, sizeof(char));
    stbtt_PackBegin(&pack_context, pixels, 512, 512, 512, 1, NULL);
    stbtt_PackFontRange(&pack_context, font_buffer, 0, font_size, 0, 125, internal_data->char_data);
    stbtt_PackEnd(&pack_context);

    internal_data->texture = cstrl_texture_generate_from_bitmap(pixels, 512, 512, CSTRL_RED, CSTRL_RED);

    free(pixels);
    free(font_buffer);
    return data;
}

CSTRL_API void cstrl_font_renderer_set_position(cstrl_font_data *font_data, int text_block, float x, float y)
{
    internal_data *internal_data = font_data->internal_data;
    float original_x = internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][text_block * TEXT_BLOCK_SIZE * 12];
    float original_y = internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][text_block * TEXT_BLOCK_SIZE * 12 + 3];
    for (int i = text_block * TEXT_BLOCK_SIZE * 6; i < (text_block + 1) * TEXT_BLOCK_SIZE * 6; i++)
    {
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][i * 2] -= original_x;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][i * 2] += x;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][i * 2 + 1] -= original_y;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][i * 2 + 1] += y;
    }
}

CSTRL_API void cstrl_font_renderer_set_color(cstrl_font_data *font_data, int text_block, float r, float g, float b,
                                             float a)
{
    internal_data *internal_data = font_data->internal_data;
    for (int i = text_block * TEXT_BLOCK_SIZE * 6; i < (text_block + 1) * TEXT_BLOCK_SIZE * 6; i++)
    {
        internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][i * 4] = r;
        internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][i * 4 + 1] = g;
        internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][i * 4 + 2] = b;
        internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][i * 4 + 3] = a;
    }
}

CSTRL_API void cstrl_font_renderer_set_text(cstrl_font_data *font_data, int text_block, const char *text, float x,
                                            float y, float r, float g, float b, float a)
{
    internal_data *internal_data = font_data->internal_data;
    memset(internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS] + text_block * TEXT_BLOCK_SIZE * 24, 0,
           TEXT_BLOCK_SIZE * 24);
    int next_x = x;
    int start_y = y;
    for (int i = 0; i < cstrl_min(TEXT_BLOCK_SIZE, strlen(text)); i++)
    {
        int index = text_block * TEXT_BLOCK_SIZE + i;
        float x0 = (float)next_x + internal_data->char_data[text[i]].xoff;
        float y0 = internal_data->font_size / 2.0f + (float)start_y + internal_data->char_data[text[i]].yoff2;
        float x1 = (float)next_x + (internal_data->char_data[text[i]].xoff2 - internal_data->char_data[text[i]].xoff);
        float y1 = internal_data->font_size / 2.0f + (float)start_y + internal_data->char_data[text[i]].yoff;
        next_x += internal_data->char_data[text[i]].xadvance;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12] = x0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 1] = y1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 2] = x1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 3] = y0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 4] = x0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 5] = y0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 6] = x0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 7] = y1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 8] = x1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 9] = y0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 10] = x1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_POSITIONS][index * 12 + 11] = y1;

        float u0 = (float)internal_data->char_data[text[i]].x0 / 512.0f;
        float v0 = (float)internal_data->char_data[text[i]].y1 / 512.0f;
        float u1 = (float)internal_data->char_data[text[i]].x1 / 512.0f;
        float v1 = (float)internal_data->char_data[text[i]].y0 / 512.0f;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12] = u0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 1] = v1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 2] = u1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 3] = v0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 4] = u0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 5] = v0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 6] = u0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 7] = v1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 8] = u1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 9] = v0;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 10] = u1;
        internal_data->font_buffers[FONT_BUFFER_TYPE_UVS][index * 12 + 11] = v1;

        for (int j = 0; j < 6; j++)
        {
            internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][index * 24 + j * 4] = r;
            internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][index * 24 + j * 4 + 1] = g;
            internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][index * 24 + j * 4 + 2] = b;
            internal_data->font_buffers[FONT_BUFFER_TYPE_COLORS][index * 24 + j * 4 + 3] = a;
        }
    }
}

CSTRL_API void cstrl_font_renderer_draw(cstrl_font_data *font_data, int block_count, mat4 view, mat4 projection)
{
    bool depth_test_enabled = cstrl_renderer_depth_test_enabled();
    cstrl_renderer_set_depth_test_enabled(false);
    internal_data *internal_data = font_data->internal_data;
    cstrl_set_uniform_mat4(internal_data->shader.program, "view", view);
    cstrl_set_uniform_mat4(internal_data->shader.program, "projection", projection);
    cstrl_texture_bind(internal_data->texture);
    cstrl_renderer_draw_custom_count(internal_data->render_data, block_count * TEXT_BLOCK_SIZE * 6);
    cstrl_renderer_set_depth_test_enabled(depth_test_enabled);
}

CSTRL_API void cstrl_font_renderer_free(cstrl_font_data *font_data)
{
    if (font_data == NULL)
    {
        return;
    }
    internal_data *internal_data = font_data->internal_data;
    cstrl_renderer_unmap_positions_range(internal_data->render_data);
    cstrl_renderer_unmap_uvs_range(internal_data->render_data);
    cstrl_renderer_unmap_colors_range(internal_data->render_data);
    cstrl_renderer_free_render_data(internal_data->render_data);
    free(internal_data->char_data);
    free(internal_data);
    free(font_data);
}
