//
// Created by sterling on 12/5/24.
//

#ifndef CSTRL_UI_H
#define CSTRL_UI_H

#include "cstrl_platform.h"

#define GEN_ID(group) (((group) * 10000) + (__LINE__))

typedef struct cstrl_ui_context
{
    void *internal_ui_state;
    void *internal_render_state;
} cstrl_ui_context;

typedef enum cstrl_ui_text_alignment
{
    CSTRL_UI_TEXT_ALIGN_LEFT,
    CSTRL_UI_TEXT_ALIGN_CENTER,
    CSTRL_UI_TEXT_ALIGN_RIGHT
} cstrl_ui_text_alignment;

CSTRL_API void cstrl_ui_init(cstrl_ui_context *context, cstrl_platform_state *platform_state);

CSTRL_API void cstrl_ui_begin(cstrl_ui_context *context);

CSTRL_API void cstrl_ui_end(cstrl_ui_context *context);

CSTRL_API void cstrl_ui_shutdown(cstrl_ui_context *context);

CSTRL_API bool cstrl_ui_region_hit(int test_x, int test_y, int object_x, int object_y, int object_width, int object_height);

CSTRL_API float cstrl_ui_text_width(cstrl_ui_context *context, const char *text, float scale);

CSTRL_API bool cstrl_ui_container_begin(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w,
                              int h, int id, bool is_static, bool can_minimize, int order_priority);

CSTRL_API void cstrl_ui_container_end(cstrl_ui_context *context);

CSTRL_API bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w, int h,
                     int id);

CSTRL_API bool cstrl_ui_text(cstrl_ui_context *context, const char *text, int title_length, int x, int y, int w, int h,
                   int id, cstrl_ui_text_alignment alignment);

CSTRL_API bool cstrl_ui_text_field(cstrl_ui_context *context, const char *placeholder, int placeholder_length, int x, int y,
                         int w, int h, int id, char *buffer, size_t buffer_size);

CSTRL_API bool cstrl_ui_mouse_locked(cstrl_ui_context *context);

#endif // CSTRL_UI_H
