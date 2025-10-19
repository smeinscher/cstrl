#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_nuklear_ui.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "glad/glad.h"
#include "log.c/log.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_API CSTRL_API
#include <nuklear/nuklear.h>

#define REFERENCE_WIDTH 1920
#define REFERENCE_HEIGHT 1080

// TODO: determine if user should calc these values
#define MAX_VERTEX_BUFFER (512 * 512)
#define MAX_ELEMENT_BUFFER (128 * 512)

#define CSTRL_NK_TEXT_MAX 256

typedef struct cstrl_nk_vertex
{
    float position[2];
    float uv[2];
    float color[4];
} cstrl_nk_vertex;

struct nk_draw_null_texture g_null_texture;

#define NK_SHADER_VERSION "#version 450 core\n"
#define NK_SHADER_BINDLESS "#extension GL_ARB_bindless_texture : require\n"

static cstrl_render_data *g_render_data;
static cstrl_shader g_shader;
static cstrl_texture g_font_texture;
static struct nk_font_atlas g_atlas;
static struct nk_buffer g_cmds;
static unsigned int vao, vbo, ebo;
static cstrl_nk_vertex *g_vertex_buffer;
static int *g_element_buffer;
static unsigned int g_text[CSTRL_NK_TEXT_MAX];
static int g_text_length;
static struct nk_vec2 g_scroll;

// Mostly copied from nk_glfw3_device_create in nuklear_glfw_gl4.h
static void opengl_create()
{
    GLint status;
    GLint len = 0;
    static const GLchar *vertex_shader = NK_SHADER_VERSION "uniform mat4 projection;\n"
                                                           "in vec2 a_pos;\n"
                                                           "in vec2 a_uv;\n"
                                                           "in vec4 a_color;\n"
                                                           "out vec2 uv;\n"
                                                           "out vec4 color;\n"
                                                           "void main() {\n"
                                                           "   uv = a_uv;\n"
                                                           "   color = a_color;\n"
                                                           "   gl_Position = projection * vec4(a_pos, 0, 1);\n"
                                                           "}\n";
    static const GLchar *fragment_shader = NK_SHADER_VERSION "precision mediump float;\n"
                                                             "uniform sampler2D texture0;\n"
                                                             "in vec2 uv;\n"
                                                             "in vec4 color;\n"
                                                             "out vec4 frag_color;\n"
                                                             "void main(){\n"
                                                             "   frag_color = color * texture(texture0, uv);\n"
                                                             "}\n";

    g_shader = cstrl_load_shaders_from_source(vertex_shader, fragment_shader);
    nk_buffer_init_default(&g_cmds);
    GLint attrib_pos = glGetAttribLocation(g_shader.program, "a_pos");
    GLint attrib_uv = glGetAttribLocation(g_shader.program, "a_uv");
    GLint attrib_col = glGetAttribLocation(g_shader.program, "a_color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(cstrl_nk_vertex);
        size_t vp = offsetof(cstrl_nk_vertex, position);
        size_t vt = offsetof(cstrl_nk_vertex, uv);
        size_t vc = offsetof(cstrl_nk_vertex, color);

        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glGenVertexArrays(1, &vao);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glEnableVertexAttribArray((GLuint)attrib_pos);
        glEnableVertexAttribArray((GLuint)attrib_uv);
        glEnableVertexAttribArray((GLuint)attrib_col);

        glVertexAttribPointer((GLuint)attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void *)vp);
        glVertexAttribPointer((GLuint)attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void *)vt);
        glVertexAttribPointer((GLuint)attrib_col, 4, GL_FLOAT, GL_TRUE, vs, (void *)vc);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    unsigned char white_pixel[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    g_null_texture.texture = nk_handle_id(texture);
    g_null_texture.uv = nk_vec2(0.0f, 0.0f);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_nuklear_init(struct nk_context *context, cstrl_platform_state *platform_state)
{
    opengl_create();

    nk_init_default(context, 0);
}

static void upload_atlas(const void *image, int width, int height)
{
    g_font_texture = cstrl_texture_generate_from_bitmap((unsigned char *)image, width, height, CSTRL_RGBA, CSTRL_RGBA);
}

CSTRL_API void cstrl_nuklear_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&g_atlas);
    nk_font_atlas_begin(&g_atlas);
    *atlas = &g_atlas;
}

CSTRL_API void cstrl_nuklear_font_stash_end(struct nk_context *context)
{
    const void *image;
    int width, height;
    image = nk_font_atlas_bake(&g_atlas, &width, &height, NK_FONT_ATLAS_RGBA32);
    upload_atlas(image, width, height);
    nk_font_atlas_end(&g_atlas, nk_handle_id((int)g_font_texture.id), &g_null_texture);
    if (g_atlas.default_font)
    {
        nk_style_set_font(context, &g_atlas.default_font->handle);
    }
}

static void new_frame(struct nk_context *context, cstrl_platform_state *platform_state)
{
    nk_input_begin(context);

    for (int i = 0; i < g_text_length; i++)
    {
        nk_input_unicode(context, g_text[i]);
    }

    int mouse_x, mouse_y;
    cstrl_platform_get_cursor_position(platform_state, &mouse_x, &mouse_y);
    nk_input_motion(context, mouse_x, mouse_y);

    nk_input_button(context, NK_BUTTON_LEFT, mouse_x, mouse_y,
                    cstrl_platform_is_mouse_button_down(platform_state, CSTRL_MOUSE_BUTTON_LEFT));
    nk_input_button(context, NK_BUTTON_RIGHT, mouse_x, mouse_y,
                    cstrl_platform_is_mouse_button_down(platform_state, CSTRL_MOUSE_BUTTON_RIGHT));
    nk_input_button(context, NK_BUTTON_MIDDLE, mouse_x, mouse_y,
                    cstrl_platform_is_mouse_button_down(platform_state, CSTRL_MOUSE_BUTTON_MIDDLE));
    nk_input_scroll(context, g_scroll);

    nk_input_end(context);
    g_text_length = 0;
    g_scroll = nk_vec2(0.0f, 0.0f);
}

CSTRL_API void cstrl_nuklear_start(struct nk_context *context, cstrl_platform_state *platform_state)
{
    new_frame(context, platform_state);
}

static void render(struct nk_context *context, cstrl_platform_state *platform_state)
{
    bool depth_test_enabled = cstrl_renderer_depth_test_enabled();
    bool cull_face_enabled = cstrl_renderer_cull_face_enabled();
    cstrl_renderer_set_depth_test_enabled(false);
    cstrl_renderer_set_cull_face_enabled(false);
    glEnable(GL_SCISSOR_TEST);
    cstrl_use_shader(g_shader);
    mat4 ortho = {{2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f}};
    int width, height;
    cstrl_platform_get_viewport_size(platform_state, &width, &height);
    ortho.xx /= (GLfloat)width;
    ortho.yy /= (GLfloat)height;
    cstrl_set_uniform_mat4(g_shader.program, "projection", ortho);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_BUFFER, NULL, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_BUFFER, NULL, GL_STREAM_DRAW);

    void *vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    void *elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    struct nk_convert_config config = {0};
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(cstrl_nk_vertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(cstrl_nk_vertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R32G32B32A32_FLOAT, NK_OFFSETOF(cstrl_nk_vertex, color)},
        {NK_VERTEX_LAYOUT_END}};
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(cstrl_nk_vertex);
    config.vertex_alignment = NK_ALIGNOF(cstrl_nk_vertex);
    config.tex_null = g_null_texture;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    // TODO: user determines AA
    config.shape_AA = true;
    config.line_AA = true;
    struct nk_buffer vertex_buffer, element_buffer;
    nk_buffer_init_fixed(&vertex_buffer, vertices, MAX_VERTEX_BUFFER);
    nk_buffer_init_fixed(&element_buffer, elements, MAX_ELEMENT_BUFFER);
    nk_convert(context, &g_cmds, &vertex_buffer, &element_buffer, &config);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    const struct nk_draw_command *cmd;
    nk_size offset = 0;
    nk_draw_foreach(cmd, context, &g_cmds)
    {
        if (!cmd->elem_count)
        {
            continue;
        }

        glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
        glScissor((GLint)cmd->clip_rect.x, (GLint)height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h),
                  (GLint)cmd->clip_rect.w, (GLint)cmd->clip_rect.h);
        glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, (const void *)offset);
        offset += cmd->elem_count * sizeof(nk_draw_index);
    }
    nk_clear(context);
    nk_buffer_clear(&g_cmds);

    cstrl_renderer_set_depth_test_enabled(depth_test_enabled);
    cstrl_renderer_set_cull_face_enabled(cull_face_enabled);
    // TODO: do this in renderer
    glDisable(GL_SCISSOR_TEST);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

CSTRL_API void cstrl_nuklear_end(struct nk_context *context, cstrl_platform_state *platform_state)
{
    render(context, platform_state);
}

CSTRL_API void cstrl_nuklear_shutdown(struct nk_context *context)
{
    nk_free(context);
}
