//
// Created by sterling on 12/5/24.
//

#ifndef CSTRL_UI_H
#define CSTRL_UI_H

#include "cstrl/cstrl_renderer.h"
#include "cstrl_platform.h"

#define GEN_ID(group) (((group) * 10000) + (__LINE__))

typedef struct cstrl_ui_context
{
    void *internal_ui_state;
} cstrl_ui_context;

typedef enum cstrl_ui_text_alignment
{
    CSTRL_UI_TEXT_ALIGN_LEFT,
    CSTRL_UI_TEXT_ALIGN_CENTER,
    CSTRL_UI_TEXT_ALIGN_RIGHT
} cstrl_ui_text_alignment;

cstrl_ui_context *cstrl_ui_init(cstrl_platform_state *platform_state);

void cstrl_ui_begin(cstrl_ui_context *context);

void cstrl_ui_end(cstrl_ui_context *context);

void cstrl_ui_shutdown(cstrl_ui_context *context);

bool cstrl_ui_region_hit(int test_x, int test_y, int object_x, int object_y, int object_width, int object_height);

float cstrl_ui_text_width(cstrl_ui_context *context, const char *text, float scale);

bool cstrl_ui_text(cstrl_ui_context *context, const char *text, int title_length, int x, int y, int w, int h,
                   cstrl_ui_text_alignment alignment);

bool cstrl_ui_container_begin(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w,
                              int h, int id, bool is_static, int order_priority);

void cstrl_ui_container_end(cstrl_ui_context *context);

bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w, int h);

#endif // CSTRL_UI_H
