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
    cstrl_render_data *font_render_data;
    cstrl_shader font_shader;
    cstrl_texture font_texture;
} cstrl_ui_context;

cstrl_ui_context *cstrl_ui_init(cstrl_platform_state *platform_state);

void cstrl_ui_begin(cstrl_ui_context *context);

void cstrl_ui_end(cstrl_ui_context *context);

void cstrl_ui_shutdown(cstrl_ui_context *context);

bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w, int h);

bool cstrl_ui_menu_bar(cstrl_ui_context *context);

#endif // CSTRL_UI_H
