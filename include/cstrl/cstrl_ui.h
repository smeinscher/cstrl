//
// Created by sterling on 12/5/24.
//

#ifndef CSTRL_UI_H
#define CSTRL_UI_H

#include "cstrl/cstrl_renderer.h"
#include "cstrl_platform.h"

typedef struct cstrl_ui_context
{
    cstrl_platform_state *platform_state;
    cstrl_render_data *render_data;
    cstrl_shader shader;
    cstrl_texture texture;
} cstrl_ui_context;

cstrl_ui_context *cstrl_ui_init(cstrl_platform_state *platform_state);

void cstrl_ui_begin(cstrl_ui_context *context);

void cstrl_ui_end(cstrl_ui_context *context);

void cstrl_ui_shutdown(cstrl_ui_context *context);

#endif // CSTRL_UI_H
