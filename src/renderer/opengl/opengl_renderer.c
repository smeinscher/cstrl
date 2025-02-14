//
// Created by 12105 on 11/23/2024.
//

#include "cstrl/cstrl_platform.h"
#if defined(CSTRL_RENDER_API_OPENGL)
#include "cstrl/cstrl_defines.h"
#if defined(CSTRL_PLATFORM_ANDROID)
#include <glad/gles3/glad.h>
#else
#include "glad/glad.h"
#endif
#include "log.c/log.h"
#include "opengl_platform.h"

#include <cstrl/cstrl_renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct internal_data
{
    unsigned int vao;
    unsigned int vbos[4];
    unsigned int ebo;
    size_t count;
    unsigned int dimensions;
    size_t indices_count;
    bool cleared;
} internal_data;

CSTRL_API bool cstrl_renderer_init(cstrl_platform_state *platform_state)
{
    if (!cstrl_opengl_platform_init(platform_state))
    {
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    glPatchParameteri(GL_PATCH_VERTICES, 16);

    return true;
}

CSTRL_API void cstrl_renderer_clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

CSTRL_API void cstrl_renderer_set_viewport(int x, int y, unsigned int width, unsigned int height)
{
    glViewport(x, y, width, height);
}

CSTRL_API cstrl_render_data *cstrl_renderer_create_render_data()
{
    cstrl_render_data *data = malloc(sizeof(cstrl_render_data));
    data->internal_data = malloc(sizeof(internal_data));
    internal_data *internal_data = data->internal_data;

    internal_data->vao = 0;
    internal_data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS] = 0;
    internal_data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS] = 0;
    internal_data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS] = 0;
    internal_data->vbos[CSTRL_RENDER_ATTRIBUTE_NORMALS] = 0;
    internal_data->ebo = 0;
    internal_data->count = 0;
    internal_data->dimensions = 2;
    internal_data->indices_count = 0;
    internal_data->cleared = false;

    glGenVertexArrays(1, &internal_data->vao);

    return data;
}

CSTRL_API void cstrl_renderer_free_render_data(cstrl_render_data *render_data)
{
    internal_data *internal_data = render_data->internal_data;
    glDeleteVertexArrays(1, &internal_data->vao);
    glDeleteBuffers(1, &internal_data->vbos[0]);
    glDeleteBuffers(1, &internal_data->vbos[1]);
    glDeleteBuffers(1, &internal_data->vbos[2]);
    glDeleteBuffers(1, &internal_data->ebo);
    free(render_data->internal_data);
    free(render_data);
}

CSTRL_API void cstrl_renderer_add_positions(cstrl_render_data *render_data, float *positions, unsigned int dimensions,
                                            unsigned int vertex_count)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;
    data->dimensions = dimensions;
    data->count = vertex_count;

    glGenBuffers(1, &data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * dimensions * sizeof(float), positions, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, dimensions * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_add_uvs(cstrl_render_data *render_data, float *uvs)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;
    glGenBuffers(1, &data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 2 * sizeof(float), uvs, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_add_colors(cstrl_render_data *render_data, float *colors)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    glGenBuffers(1, &data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 4 * sizeof(float), colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_add_normals(cstrl_render_data *render_data, float *normals)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    glGenBuffers(1, &data->vbos[CSTRL_RENDER_ATTRIBUTE_NORMALS]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_NORMALS]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 3 * sizeof(float), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_add_indices(cstrl_render_data *render_data, int *indices, size_t indices_count)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;
    data->indices_count = indices_count;
    glGenBuffers(1, &data->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_count * sizeof(int), indices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_modify_positions(cstrl_render_data *render_data, float *positions, size_t start_index,
                                               size_t count)
{
    if (count == 0)
    {
        log_warn("Modifying positions with 0 count, skipping to avoid possible undefined behavior");
        return;
    }
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    if (positions != NULL)
    {
        glBindVertexArray(data->vao);
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS]);
        if (count + start_index > data->count * data->dimensions)
        {
            data->count = (count + start_index) / data->dimensions;
            glBufferData(GL_ARRAY_BUFFER, data->count * data->dimensions * sizeof(float), positions, GL_DYNAMIC_DRAW);
        }
        else
        {
            glBufferSubData(GL_ARRAY_BUFFER, start_index * sizeof(float), count * sizeof(float),
                            positions + start_index);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_modify_indices(cstrl_render_data *render_data, int *indices, size_t start_index,
                                             size_t count)
{
    if (count == 0)
    {
        log_warn("Modifying indices with 0 count, skipping to avoid possible undefined behavior");
        return;
    }
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    if (indices != NULL)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
        if (start_index + count > data->indices_count)
        {
            data->indices_count = start_index + count;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->indices_count * sizeof(int), indices, GL_DYNAMIC_DRAW);
        }
        else
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start_index * sizeof(int), count * sizeof(int),
                            indices + start_index);
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_modify_render_attributes(cstrl_render_data *render_data, const float *positions,
                                                       const float *uvs, const float *colors, size_t count)
{
    if (count == 0)
    {
        log_warn("Modifying render data with 0 count, skipping to avoid possible undefined behavior");
        return;
    }
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    if (count != data->count && (positions == NULL || (uvs == NULL && data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS] != 0) ||
                                 (colors == NULL && data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS] != 0)))
    {
        log_error("When adding or removing data, need all attributes to be present");
        return;
    }
    glBindVertexArray(data->vao);
    if (uvs != NULL)
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS]);
        if (count == data->count)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, count * 2 * sizeof(float), uvs);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(float), uvs, GL_DYNAMIC_DRAW);
        }
    }
    if (colors != NULL)
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS]);
        if (count == data->count)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, count * 4 * sizeof(float), colors);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, count * 4 * sizeof(float), colors, GL_DYNAMIC_DRAW);
        }
    }
    if (positions != NULL)
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS]);
        if (count == data->count)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, count * data->dimensions * sizeof(float), positions);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, count * data->dimensions * sizeof(float), positions, GL_DYNAMIC_DRAW);
            data->count = count;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_clear_render_attributes(cstrl_render_data *render_data)
{
    internal_data *data = render_data->internal_data;
    if (data->cleared)
    {
        return;
    }
    data->cleared = true;
    data->count = 0;
    float zero_float = 0.0f;
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS]);
    glClearBufferData(GL_ARRAY_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &zero_float);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS]);
    glClearBufferData(GL_ARRAY_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &zero_float);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS]);
    glClearBufferData(GL_ARRAY_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &zero_float);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_NORMALS]);
    glClearBufferData(GL_ARRAY_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &zero_float);
    int zero_int = 0;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
    glClearBufferData(GL_ELEMENT_ARRAY_BUFFER, GL_R32I, GL_RED, GL_INT, &zero_int);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_draw(cstrl_render_data *data)
{
    internal_data *internal_data = data->internal_data;
    glBindVertexArray(internal_data->vao);
    glDrawArrays(GL_TRIANGLES, 0, internal_data->count);
}

CSTRL_API void cstrl_renderer_draw_indices(cstrl_render_data *data)
{
    internal_data *internal_data = data->internal_data;
    glBindVertexArray(internal_data->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, internal_data->ebo);
    glDrawElements(GL_TRIANGLES, internal_data->indices_count, GL_UNSIGNED_INT, 0);
}

CSTRL_API void cstrl_renderer_draw_lines(cstrl_render_data *data)
{
    internal_data *internal_data = data->internal_data;
    glBindVertexArray(internal_data->vao);
    glDrawArrays(GL_LINES, 0, internal_data->count);
}

CSTRL_API void cstrl_renderer_draw_patches(cstrl_render_data *data)
{
    internal_data *internal_data = data->internal_data;
    glBindVertexArray(internal_data->vao);
    glDrawArrays(GL_PATCHES, 0, internal_data->count);
}

CSTRL_API void cstrl_renderer_shutdown(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_destroy(platform_state);
}

CSTRL_API void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_swap_buffers(platform_state);
}

CSTRL_API void cstrl_renderer_set_depth_test_enabled(bool enabled)
{
    if (enabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

#endif
