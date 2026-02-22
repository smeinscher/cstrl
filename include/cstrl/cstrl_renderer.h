//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_RENDERER_H
#define CSTRL_RENDERER_H

#include "cstrl/cstrl_defines.h"
#include "cstrl_math.h"
#include "cstrl_platform.h"
#include <time.h>

#if defined(CSTRL_RENDER_API_OPENGL) && defined(CSTRL_PLATFORM_WINDOWS)
#include <windows.h>
CSTRL_API extern DWORD NvOptimusEnablement;
#endif

typedef enum cstrl_render_attribute_type
{
    CSTRL_RENDER_ATTRIBUTE_POSITIONS,
    CSTRL_RENDER_ATTRIBUTE_UVS,
    CSTRL_RENDER_ATTRIBUTE_COLORS,
    CSTRL_RENDER_ATTRIBUTE_NORMALS,
    CSTRL_RENDER_ATTRIBUTE_TANGENTS,
    CSTRL_RENDER_ATTRIBUTE_BITANGENTS,
    CSTRL_RENDER_ATTRIBUTE_OFFSETS,
    CSTRL_RENDER_ATTRIBUTE_MAX
} cstrl_render_attribute_type;

#if defined(CSTRL_RENDER_API_VULKAN)

typedef struct cstrl_shader
{
    int program;
} cstrl_shader;

typedef struct cstrl_texture
{
    int id;
} cstrl_texture;

#else

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

typedef enum cstrl_texture_format
{
    CSTRL_RED,
    CSTRL_RGB,
    CSTRL_RGBA
} cstrl_texture_format;

typedef CSTRL_PACKED_ENUM{CSTRL_TEXTURE_FILTER_LINEAR, CSTRL_TEXTURE_FILTER_NEAREST} cstrl_texture_filter;

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

CSTRL_API bool cstrl_renderer_init(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_renderer_clear(float r, float g, float b, float a);

CSTRL_API void cstrl_renderer_set_viewport(int x, int y, unsigned int width, unsigned int height);

CSTRL_API void cstrl_create_framebuffer(int width, int height, unsigned int *fbo, unsigned int *rbo, unsigned int *vao);

CSTRL_API void cstrl_renderer_bind_framebuffer(unsigned int fbo);

CSTRL_API void cstrl_renderer_framebuffer_draw(unsigned int vao);

CSTRL_API cstrl_render_data *cstrl_renderer_create_render_data();

CSTRL_API void cstrl_renderer_free_render_data(cstrl_render_data *render_data);

CSTRL_API void cstrl_renderer_add_positions(cstrl_render_data *render_data, float *positions, unsigned int dimensions,
                                            unsigned int vertex_count);

CSTRL_API void cstrl_renderer_add_uvs(cstrl_render_data *render_data, float *uvs);

CSTRL_API void cstrl_renderer_add_colors(cstrl_render_data *render_data, float *colors);

CSTRL_API void cstrl_renderer_add_colors_instanced(cstrl_render_data *render_data, float *colors, size_t instance_count);

CSTRL_API void cstrl_renderer_add_normals(cstrl_render_data *render_data, float *normals);

CSTRL_API void cstrl_renderer_add_indices(cstrl_render_data *render_data, int *indices, size_t indices_count);

CSTRL_API void cstrl_renderer_add_tangents(cstrl_render_data *render_data, float *tangents);

CSTRL_API void cstrl_renderer_add_bitangents(cstrl_render_data *render_data, float *bitangents);

CSTRL_API void cstrl_renderer_add_offsets_instanced(cstrl_render_data *render_data, float *offsets, size_t instance_count);

CSTRL_API void cstrl_renderer_modify_positions(cstrl_render_data *render_data, float *positions, size_t start_index,
                                               size_t count);

CSTRL_API void cstrl_renderer_modify_uvs(cstrl_render_data *render_data, float *uvs, size_t start_index, size_t count);

CSTRL_API void cstrl_renderer_modify_colors(cstrl_render_data *render_data, float *colors, size_t start_index,
                                            size_t count);

CSTRL_API void cstrl_renderer_modify_indices(cstrl_render_data *render_data, int *indices, size_t start_index,
                                             size_t count);

CSTRL_API void cstrl_renderer_modify_render_attributes(cstrl_render_data *render_data, const float *positions,
                                                       const float *uvs, const float *colors, size_t count);

CSTRL_API void cstrl_renderer_clear_render_attributes(cstrl_render_data *render_data);

CSTRL_API void cstrl_renderer_draw(cstrl_render_data *data);

CSTRL_API void cstrl_renderer_draw_indices(cstrl_render_data *data);

CSTRL_API void cstrl_renderer_draw_indices_by_count_and_offset(cstrl_render_data *data, int count, int *offset);

CSTRL_API void cstrl_renderer_draw_indices_instanced(cstrl_render_data *data, int instances);

CSTRL_API void cstrl_renderer_draw_lines(cstrl_render_data *data);

CSTRL_API void cstrl_renderer_draw_lines_indices(cstrl_render_data *data);

CSTRL_API void cstrl_renderer_draw_patches(cstrl_render_data *data);

CSTRL_API void cstrl_renderer_shutdown(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state);

CSTRL_API bool cstrl_renderer_depth_test_enabled();

CSTRL_API void cstrl_renderer_set_depth_test_enabled(bool enabled);

CSTRL_API bool cstrl_renderer_cull_face_enabled();

CSTRL_API void cstrl_renderer_set_cull_face_enabled(bool enabled);

CSTRL_API unsigned int cstrl_renderer_add_ubo(size_t size);

CSTRL_API void cstrl_renderer_update_ubo(unsigned int ubo, void *object, size_t size, size_t offset);

CSTRL_API void cstrl_renderer_set_line_width(float line_width);

CSTRL_API float *cstrl_renderer_map_positions_range(cstrl_render_data *render_data, float *data);

CSTRL_API void cstrl_renderer_unmap_positions_range(cstrl_render_data *render_data);

CSTRL_API float *cstrl_renderer_map_uvs_range(cstrl_render_data *render_data, float *data);

CSTRL_API void cstrl_renderer_unmap_uvs_range(cstrl_render_data *render_data);

CSTRL_API float *cstrl_renderer_map_colors_range(cstrl_render_data *render_data, float *data);

CSTRL_API float *cstrl_renderer_map_colors_range_instanced(cstrl_render_data *render_data, float *data, size_t size);

CSTRL_API void cstrl_renderer_unmap_colors_range(cstrl_render_data *render_data);

CSTRL_API int *cstrl_renderer_map_indices_range(cstrl_render_data *render_data, int *data);

CSTRL_API void cstrl_renderer_unmap_indices_range(cstrl_render_data *render_data);

CSTRL_API float *cstrl_renderer_map_offsets_range_instanced(cstrl_render_data *render_data, float *data, size_t size);

CSTRL_API void cstrl_renderer_unmap_offsets_range(cstrl_render_data *render_data);

/*
 *
 *  Shader
 *
 */

CSTRL_API cstrl_shader cstrl_load_shaders_from_files(const char *vertex_shader_path, const char *fragment_shader_path);

CSTRL_API cstrl_shader cstrl_load_shaders_from_source(const char *vertex_shader_source,
                                                      const char *fragment_shader_source);

CSTRL_API cstrl_shader cstrl_load_shaders_tessellation_from_files(const char *vertex_shader_path,
                                                                  const char *fragment_shader_path,
                                                                  const char *tessellation_control_shader_path,
                                                                  const char *tessellation_evaluation_shader_path);

CSTRL_API cstrl_shader cstrl_load_shaders_tessellation_from_source(const char *vertex_shader_source,
                                                                   const char *fragment_shader_source,
                                                                   const char *tessellation_control_shader_source,
                                                                   const char *tessellation_evaluation_shader_source);

CSTRL_API void cstrl_use_shader(cstrl_shader shader);

CSTRL_API int *cstrl_get_uniform_offsets(cstrl_shader shader, const char *names[], unsigned int count);

CSTRL_API void cstrl_free_uniform_offsets(int **offsets);

CSTRL_API void cstrl_set_uniform_block_binding(cstrl_shader shader, const char *name, unsigned int binding_point);

CSTRL_API void cstrl_set_uniform_int(unsigned int program, const char *name, int d);

CSTRL_API void cstrl_set_uniform_int_array(unsigned int program, const char *name, int count, int *d);

CSTRL_API void cstrl_set_uniform_float(unsigned int program, const char *name, float f);

CSTRL_API void cstrl_set_uniform_3f(unsigned int program, const char *name, float x, float y, float z);

CSTRL_API void cstrl_set_uniform_4f(unsigned int program, const char *name, float x, float y, float z, float w);

CSTRL_API void cstrl_set_uniform_mat3(unsigned int program, const char *name, mat3 mat);

CSTRL_API void cstrl_set_uniform_mat4(unsigned int program, const char *name, mat4 mat);

CSTRL_API void cstrl_shader_hot_reload(cstrl_shader *shader);

/*
 *
 *  Texture
 *
 */

CSTRL_API void cstrl_texture_set_flip_vertically(bool flip_vertically);

CSTRL_API cstrl_texture cstrl_texture_framebuffer_generate(int width, int height);

CSTRL_API cstrl_texture cstrl_texture_generate_from_path(const char *path, cstrl_texture_filter texture_filter);

CSTRL_API cstrl_texture cstrl_texture_generate_from_bitmap(unsigned char *bitmap, int width, int height,
                                                           cstrl_texture_format format,
                                                           cstrl_texture_format internal_format);

CSTRL_API cstrl_texture cstrl_texture_cube_map_generate_from_folder(const char *folder, bool alpha_channel);

CSTRL_API void cstrl_texture_hot_reload(cstrl_texture *texture);

CSTRL_API void cstrl_texture_bind(cstrl_texture texture);

CSTRL_API void cstrl_texture_cube_map_bind(cstrl_texture texture);

CSTRL_API void cstrl_set_active_texture(unsigned int active_texture);

#endif // CSTRL_RENDERER_H
