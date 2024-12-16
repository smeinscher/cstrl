//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_RENDERER_H
#define CSTRL_RENDERER_H

#include "cstrl_math.h"
#include "cstrl_platform.h"

typedef enum cstrl_render_attribute_type
{
    CSTRL_RENDER_ATTRIBUTE_POSITIONS,
    CSTRL_RENDER_ATTRIBUTE_UVS,
    CSTRL_RENDER_ATTRIBUTE_COLORS,
    CSTRL_RENDER_ATTRIBUTE_NORMALS,
    CSTRL_RENDER_ATTRIBUTE_MAX
} cstrl_render_attribute_type;

#if defined(CSTRL_RENDER_API_OPENGL)

typedef struct cstrl_shader
{
    unsigned int program;
    const char *vertex_shader_path;
    const char *fragment_shader_path;

    time_t vertex_shader_last_modified_timestamp;
    time_t fragment_shader_last_modified_timestamp;
} cstrl_shader;

typedef struct cstrl_texture
{
    unsigned int id;
    const char *path;
    time_t last_modified_timestamp;
} cstrl_texture;

#elif defined(CSTRL_RENDER_API_VULKAN)
typedef struct cstrl_shader
{
    int program;
} cstrl_shader;

typedef struct cstrl_texture
{
    int id;
} cstrl_texture;
#endif

typedef struct cstrl_render_data
{
    void *internal_data;
} cstrl_render_data;

/*
 *
 *  Render Functions
 *
 */

bool cstrl_renderer_init(cstrl_platform_state *platform_state);

void cstrl_renderer_clear(float r, float g, float b, float a);

cstrl_render_data *cstrl_renderer_create_render_data();

void cstrl_renderer_free_render_data(cstrl_render_data *render_data);

void cstrl_renderer_add_positions(cstrl_render_data *render_data, float *positions, unsigned int dimensions,
                                  unsigned int vertex_count);

void cstrl_renderer_add_uvs(cstrl_render_data *render_data, float *uvs);

void cstrl_renderer_add_colors(cstrl_render_data *render_data, float *colors);

void cstrl_renderer_add_normals(cstrl_render_data *render_data, float *normals);

void cstrl_renderer_modify_render_attributes(cstrl_render_data *render_data, const float *positions, const float *uvs,
                                             const float *colors, size_t count);

void cstrl_renderer_draw(cstrl_render_data *data);

void cstrl_renderer_destroy(cstrl_platform_state *platform_state);

void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state);

/*
 *
 *  Shader
 *
 */

cstrl_shader cstrl_load_shaders_from_files(const char *vertex_shader_path, const char *fragment_shader_path);

cstrl_shader cstrl_load_shaders_from_source(const char *vertex_shader_source, const char *fragment_shader_source);

void cstrl_use_shader(cstrl_shader shader);

void cstrl_set_uniform_float(unsigned int program, const char *name, float f);

void cstrl_set_uniform_3f(unsigned int program, const char *name, float x, float y, float z);

void cstrl_set_uniform_4f(unsigned int program, const char *name, float x, float y, float z, float w);

void cstrl_set_uniform_mat4(unsigned int program, const char *name, mat4 mat);

void cstrl_shader_hot_reload(cstrl_shader *shader);

/*
 *
 *  Texture
 *
 */

cstrl_texture cstrl_texture_generate_from_path(const char *path);

cstrl_texture cstrl_texture_generate_from_bitmap(unsigned char *bitmap, int width, int height);

void cstrl_texture_hot_reload(cstrl_texture *texture);

void cstrl_texture_bind(cstrl_texture texture);

#endif // CSTRL_RENDERER_H
