//
// Created by 12105 on 11/23/2024.
//

#include "cstrl/cstrl_util.h"
#include "glad/glad.h"
#include "log.c/log.h"
#include "opengl_platform.h"

#include <cstrl/cstrl_renderer.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct internal_data
{
    unsigned int vao;
    unsigned int vbos[4];
    unsigned int ebo;
    size_t count;
    unsigned int dimensions;
    size_t indices_count;
    float *positions;
    float *uvs;
    float *colors;
    float *normals;
    unsigned int *indices;
} internal_data;

void cstrl_renderer_init(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_init(platform_state);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_WIDTH);
    glLineWidth(1.0f);
}

void cstrl_renderer_clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

cstrl_render_data *cstrl_renderer_create_render_data()
{
    cstrl_render_data *data = malloc(sizeof(cstrl_render_data));
    data->internal_data = malloc(sizeof(internal_data));
    internal_data *internal_data = data->internal_data;

    internal_data->vao = 0;
    internal_data->vbos[0] = 0;
    internal_data->vbos[1] = 0;
    internal_data->vbos[2] = 0;
    internal_data->ebo = 0;
    internal_data->count = 0;
    internal_data->dimensions = 2;
    internal_data->indices_count = 0;
    internal_data->positions = NULL;
    internal_data->uvs = NULL;
    internal_data->colors = NULL;
    internal_data->normals = NULL;
    internal_data->indices = NULL;

    glGenVertexArrays(1, &internal_data->vao);

    return data;
}

void cstrl_renderer_free_render_data(cstrl_render_data *render_data)
{
    internal_data *internal_data = render_data->internal_data;
    glDeleteVertexArrays(1, &internal_data->vao);
    glDeleteBuffers(1, &internal_data->vbos[0]);
    glDeleteBuffers(1, &internal_data->vbos[1]);
    glDeleteBuffers(1, &internal_data->vbos[2]);
    glDeleteBuffers(1, &internal_data->ebo);
    free(internal_data->positions);
    free(internal_data->uvs);
    free(internal_data->colors);
    free(internal_data->normals);
    free(render_data->internal_data);
    free(render_data);
}

void cstrl_renderer_add_positions(cstrl_render_data *render_data, float *positions, unsigned int dimensions,
                                  unsigned int vertex_count)
{
    internal_data *data = render_data->internal_data;
    data->positions = malloc(vertex_count * dimensions * sizeof(float));
    if (!data->positions)
    {
        log_error("Failed to allocate memory for positions");
        return;
    }
    for (int i = 0; i < vertex_count * dimensions; i++)
    {
        data->positions[i] = positions[i];
    }
    data->dimensions = dimensions;
    data->count = vertex_count;

    glGenBuffers(1, &data->vbos[0]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * dimensions * sizeof(float), data->positions, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, dimensions * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_add_uvs(cstrl_render_data *render_data, float *uvs)
{
    internal_data *data = render_data->internal_data;
    data->uvs = malloc(data->count * 2 * sizeof(float));
    if (!data->uvs)
    {
        log_error("Failed to allocate memory for uvs");
        return;
    }
    // TODO: we don't know the size of uvs, this could be dangerous
    for (int i = 0; i < data->count * 2; i++)
    {
        data->uvs[i] = uvs[i];
    }
    glGenBuffers(1, &data->vbos[1]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 2 * sizeof(float), data->uvs, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_add_colors(cstrl_render_data *render_data, float *colors)
{
    internal_data *data = render_data->internal_data;
    data->colors = malloc(data->count * 4 * sizeof(float));
    if (!data->colors)
    {
        log_error("Failed to allocate memory for colors");
        return;
    }
    // TODO: we don't know the size of colors, this could be dangerous
    for (int i = 0; i < data->count * 4; i++)
    {
        data->colors[i] = colors[i];
    }

    glGenBuffers(1, &data->vbos[2]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 4 * sizeof(float), data->colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_add_normals(cstrl_render_data *render_data, float *normals)
{
    internal_data *data = render_data->internal_data;
    data->normals = malloc(data->count * data->dimensions * sizeof(float));
    if (!data->normals)
    {
        log_error("Failed to allocate memory for normals");
        return;
    }
    // TODO: we don't know the size of normals, this could be dangerous
    for (int i = 0; i < data->count * data->dimensions; i++)
    {
        data->normals[i] = normals[i];
    }

    glGenBuffers(1, &data->vbos[3]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[3]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 3 * sizeof(float), data->normals, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_modify_render_attributes(cstrl_render_data *render_data, const float *positions, const float *uvs,
                                             const float *colors, size_t count, size_t start_index)
{
    internal_data *data = render_data->internal_data;

    if (start_index + count > data->count &&
        (positions == NULL || (uvs == NULL && data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS] != 0) ||
         (colors == NULL && data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS] != 0)))
    {
        log_error("In renderer: When adding more data, need all attributes to be present");
        return;
    }
    glBindVertexArray(data->vao);
    if (uvs != NULL)
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_UVS]);
        if (start_index + count == data->count)
        {
            for (size_t i = start_index; i < data->count * 2; i++)
            {
                data->uvs[i] = uvs[i - start_index];
            }
            glBufferSubData(GL_ARRAY_BUFFER, start_index * 2 * sizeof(float), count * 2 * sizeof(float), data->uvs);
        }
        else
        {
            cstrl_realloc_float(&data->uvs, (start_index + count) * 2);
            for (size_t i = start_index; i < start_index + count * 2; i++)
            {
                data->uvs[i] = data->uvs[i - start_index];
            }
            glBufferData(GL_ARRAY_BUFFER, (start_index + count) * 2 * sizeof(float), data->uvs, GL_DYNAMIC_DRAW);
        }
    }
    if (colors != NULL)
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_COLORS]);
        if (start_index + count == data->count)
        {
            for (size_t i = start_index; i < data->count * 4; i++)
            {
                data->colors[i] = colors[i - start_index];
            }
            glBufferSubData(GL_ARRAY_BUFFER, start_index * 4 * sizeof(float), count * 4 * sizeof(float), data->colors);
        }
        else
        {
            cstrl_realloc_float(&data->colors, (start_index + count) * 4);
            for (size_t i = start_index; i < start_index + count * 4; i++)
            {
                data->colors[i] = data->colors[i - start_index];
            }
            glBufferData(GL_ARRAY_BUFFER, (start_index + count) * 4 * sizeof(float), data->colors, GL_DYNAMIC_DRAW);
        }
    }
    if (positions != NULL)
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_POSITIONS]);
        if (start_index + count == data->count)
        {
            for (size_t i = start_index; i < data->count * data->dimensions; i++)
            {
                data->positions[i] = positions[i - start_index];
            }
            glBufferSubData(GL_ARRAY_BUFFER, start_index * data->dimensions * sizeof(float),
                            count * data->dimensions * sizeof(float), data->positions);
        }
        else
        {
            cstrl_realloc_float(&data->positions, (start_index + count) * data->dimensions);
            for (size_t i = start_index; i < start_index + count * data->dimensions; i++)
            {
                data->positions[i] = data->positions[i - start_index];
            }
            glBufferData(GL_ARRAY_BUFFER, (start_index + count) * data->dimensions * sizeof(float), data->positions,
                         GL_DYNAMIC_DRAW);
            data->count = start_index + count;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_draw(cstrl_render_data *data)
{
    internal_data *internal_data = data->internal_data;
    glBindVertexArray(internal_data->vao);
    glDrawArrays(GL_TRIANGLES, 0, internal_data->count);
}

void cstrl_renderer_destroy(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_destroy(platform_state);
}

void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_swap_buffers(platform_state);
}
