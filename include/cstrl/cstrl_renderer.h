//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_RENDERER_H
#define CSTRL_RENDERER_H
#include "cstrl_platform.h"

typedef struct render_data
{
    void *internal_data;
} render_data;

void cstrl_renderer_init(cstrl_platform_state *platform_state);

void cstrl_renderer_clear(float r, float g, float b, float a);

render_data *cstrl_renderer_create_render_data();

void cstrl_renderer_add_positions(render_data *render_data, float *positions, unsigned int dimensions,
                                  unsigned int vertex_count);

void cstrl_renderer_add_uvs(render_data *render_data, float *uvs);

void cstrl_renderer_add_colors(render_data *render_data, float *colors);

void cstrl_renderer_draw(render_data *data);

void cstrl_renderer_destroy(cstrl_platform_state *platform_state);

void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state);

#endif // CSTRL_RENDERER_H
