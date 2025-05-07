//
// Created by 12105 on 11/23/2024.
//

#include "cstrl/cstrl_assert.h"
#include "cstrl/cstrl_platform.h"
#if defined(CSTRL_RENDER_API_OPENGL)
#include "cstrl/cstrl_defines.h"
#if defined(CSTRL_PLATFORM_ANDROID) || defined(CSTRL_PLATFORM_EM_WEB)
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
    unsigned int vbos[CSTRL_RENDER_ATTRIBUTE_MAX];
    unsigned int ebo;
    unsigned int ubo;
    size_t count;
    unsigned int dimensions;
    size_t indices_count;
    bool cleared;
} internal_data;

#if defined(CSTRL_DEBUG)
static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                           const GLchar *message, const void *param)
{
    // TODO: convert enums to strings
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        log_error("source: %d, type: %d, id: %d. %s", source, type, id, message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        log_warn("source: %d, type: %d, id: %d. %s", source, type, id, message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        log_debug("source: %d, type: %d, id: %d. %s", source, type, id, message);
        break;
    default:
        log_info("source: %d, type: %d, id: %d. %s", source, type, id, message);
        break;
    }
}
#endif

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
#if !defined(CSTRL_PLATFORM_ANDROID) && !defined(CSTRL_PLATFORM_EM_WEB)
    glEnable(GL_LINE_SMOOTH);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
#endif
#if defined(CSTRL_DEBUG)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_callback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
    glLineWidth(1.0f);

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

CSTRL_API void cstrl_create_framebuffer(int width, int height, unsigned int *fbo, unsigned int *rbo, unsigned int *vao)
{
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glGenRenderbuffers(1, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, *rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *rbo);

    unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    CSTRL_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Error creating framebuffer object");

    float x0 = -1.0f;
    float y0 = -1.0f;
    float x1 = 1.0f;
    float y1 = 1.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;

    float vertices[] = {x0, y1, u0, v1, x1, y0, u1, v0, x0, y0, u0, v0, x0, y1, u0, v1, x1, y0, u1, v0, x1, y1, u1, v1};
    unsigned int vbo;
    glGenVertexArrays(1, vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

CSTRL_API void cstrl_renderer_bind_framebuffer(unsigned int fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

CSTRL_API void cstrl_renderer_framebuffer_draw(unsigned int vao)
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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

CSTRL_API void cstrl_renderer_add_tangents(cstrl_render_data *render_data, float *tangents)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    glGenBuffers(1, &data->vbos[CSTRL_RENDER_ATTRIBUTE_TANGENTS]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_TANGENTS]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 3 * sizeof(float), tangents, GL_STATIC_DRAW);
    glVertexAttribPointer(CSTRL_RENDER_ATTRIBUTE_TANGENTS, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(CSTRL_RENDER_ATTRIBUTE_TANGENTS);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CSTRL_API void cstrl_renderer_add_bitangents(cstrl_render_data *render_data, float *bitangents)
{
    internal_data *data = render_data->internal_data;
    data->cleared = false;

    glGenBuffers(1, &data->vbos[CSTRL_RENDER_ATTRIBUTE_BITANGENTS]);
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[CSTRL_RENDER_ATTRIBUTE_BITANGENTS]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 3 * sizeof(float), bitangents, GL_STATIC_DRAW);
    glVertexAttribPointer(CSTRL_RENDER_ATTRIBUTE_BITANGENTS, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(CSTRL_RENDER_ATTRIBUTE_BITANGENTS);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
            glBufferSubData(GL_ARRAY_BUFFER, start_index * sizeof(float), count * sizeof(float), positions);
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
#if !defined(CSTRL_PLATFORM_ANDROID) && !defined(CSTRL_PLATFORM_EM_WEB)
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
#else
    log_error("Can not clear render data on android/web (yet)");
#endif
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

CSTRL_API void cstrl_renderer_set_cull_face_enabled(bool enabled)
{
    if (enabled)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

CSTRL_API unsigned int cstrl_renderer_add_ubo(size_t size)
{
    unsigned int ubo;
    glGenBuffers(1, &ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, size);

    return ubo;
}

CSTRL_API void cstrl_renderer_update_ubo(unsigned int ubo, void *object, size_t size, size_t offset)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, object);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

#endif
