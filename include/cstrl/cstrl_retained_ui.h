//
// Created by sterling on 12/5/24.
//

#ifndef CSTRL_RETAINED_UI_H
#define CSTRL_RETAINED_UI_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_util.h"
#include "cstrl_platform.h"

typedef struct cstrl_ui_color_t
{
    float r, g, b, a;
} cstrl_retained_ui_color_t;
typedef cstrl_retained_ui_color_t ui_color_t;

typedef CSTRL_PACKED_ENUM{CSTRL_RETAINED_UI_PANEL,    CSTRL_RETAINED_UI_BUTTON,
                          CSTRL_RETAINED_UI_LABEL,    CSTRL_RETAINED_UI_IMAGE,
                          CSTRL_RETAINED_UI_PROGRESS, CSTRL_RETAINED_UI_MAX_TYPE} cstrl_retained_ui_type_t;
typedef cstrl_retained_ui_type_t ui_type_t;

typedef CSTRL_PACKED_ENUM{CSTRL_RETAINED_UI_XSMALL_FONT,  CSTRL_RETAINED_UI_SMALL_FONT,  CSTRL_RETAINED_UI_MEDIUM_FONT,
                          CSTRL_RETAINED_UI_LARGE_FONT,   CSTRL_RETAINED_UI_XLARGE_FONT, CSTRL_RETAINED_UI_XXLARGE_FONT,
                          CSTRL_RETAINED_UI_MAX_FONT_SIZE} cstrl_retained_ui_font_size_t;
typedef cstrl_retained_ui_font_size_t ui_font_size_t;

typedef struct cstrl_retained_ui_element_t
{
    cstrl_retained_ui_type_t _type;
    cstrl_retained_ui_font_size_t _font_size;

    float x;
    float y;
    float width;
    float height;

    float texture_u0;
    float texture_v0;
    float texture_u1;
    float texture_v1;

    int _id;
    int parent_id;
    da_int child_ids;

    char *label;
    int _label_max_length;
    int label_index;
    float label_x;
    float label_y;

    cstrl_retained_ui_color_t color;
    cstrl_retained_ui_color_t hovered_color;

    cstrl_retained_ui_color_t font_color;
    cstrl_retained_ui_color_t font_hovered_color;

    bool visible;
    bool hovered;
    bool clicked;

    void (*on_click)(struct cstrl_retained_ui_element_t *, ...);
} cstrl_retained_ui_element_t;
typedef cstrl_retained_ui_element_t ui_element_t;

CSTRL_API void cstrl_retained_ui_init(cstrl_platform_state *platform_state, const char *texture_path,
                                      const char *font_path);

CSTRL_API void cstrl_retained_ui_get_text_size(const char *text, float *width, float *height,
                                               cstrl_retained_ui_font_size_t font_size_index);

CSTRL_API cstrl_retained_ui_element_t *cstrl_retained_ui_create(cstrl_retained_ui_type_t type, char *label,
                                                                cstrl_retained_ui_font_size_t label_font_size);

CSTRL_API void cstrl_retained_ui_update(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_retained_ui_render();

CSTRL_API cstrl_retained_ui_element_t *cstrl_retained_ui_get_element_by_id(int id);

CSTRL_API bool cstrl_retained_ui_is_mouse_over_ui();

CSTRL_API void cstrl_retained_ui_shutdown();

#endif // CSTRL_RETAINED_UI_H
