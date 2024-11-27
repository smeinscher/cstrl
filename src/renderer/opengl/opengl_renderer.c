//
// Created by 12105 on 11/23/2024.
//

#include "glad/glad.h"
#include "opengl_platform.h"
#include "opengl_shader.h"

#include "../camera.h"
#include "opengl_shader_programs.h"
#include "opengl_texture.h"

#include <cstrl/cstrl_renderer.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct internal_data
{
    unsigned int vao;
    unsigned int vbos[3];
    unsigned int ebo;
    unsigned int count;
    unsigned int dimensions;
    unsigned int indices_count;
    float *positions;
    float *uvs;
    float *colors;
    unsigned int *indices;
} internal_data;

Shader shader;
Texture texture;

void cstrl_renderer_init(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_init(platform_state);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_WIDTH);
    glLineWidth(1.0f);

    shader = opengl_load_basic_shaders(basic_3d_vertex_shader, basic_3d_fragment_shader);

    camera_set_viewport_width(800);
    camera_set_viewport_height(600);
    // camera_set_position((vec3){0.0f, 0.0f, 2.0f});

    texture = generate_opengl_texture("../resources/textures/wall.jpg");
}

void cstrl_renderer_clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

render_data *cstrl_renderer_create_render_data()
{
    render_data *data = malloc(sizeof(render_data));
    data->internal_data = malloc(sizeof(internal_data));
    internal_data *internal_data = data->internal_data;

    glGenVertexArrays(1, &internal_data->vao);
    return data;
}

void cstrl_renderer_add_positions(render_data *render_data, float *positions, unsigned int dimensions,
                                  unsigned int vertex_count)
{
    internal_data *data = render_data->internal_data;
    data->positions = positions;
    data->dimensions = dimensions;
    data->count = vertex_count;

    glGenBuffers(1, &data->vbos[0]);
    glBindVertexArray(data->vao);

    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * dimensions * sizeof(float), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, dimensions * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_add_uvs(render_data *render_data, float *uvs)
{
    internal_data *data = render_data->internal_data;
    data->uvs = uvs;

    glGenBuffers(1, &data->vbos[1]);
    glBindVertexArray(data->vao);

    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, data->count * 2 * sizeof(float), uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_add_colors(render_data *render_data, float *colors)
{
    internal_data *data = render_data->internal_data;
    data->colors = colors;

    glGenBuffers(1, &data->vbos[2]);
    glBindVertexArray(data->vao);

    glBindBuffer(GL_ARRAY_BUFFER, data->vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, data->count * sizeof(float), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cstrl_renderer_draw(render_data *data)
{
    camera_update();
    glUseProgram(shader.program);
    // static float test = 0.0f;
    // opengl_set_uniform_float(shader.program, "time", (sinf(test) + 1.0f) / 2.0f);
    // test += 0.01f;
    opengl_set_uniform_mat4(shader.program, "view", camera_get_view());
    opengl_set_uniform_mat4(shader.program, "projection", camera_get_projection());
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
