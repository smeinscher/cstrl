//
// Created by sterling on 12/5/24.
//

#ifndef CSTRL_UI_H
#define CSTRL_UI_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_util.h"
#include "cstrl_platform.h"

#define GEN_ID(group) (((group) * 10000) + (__LINE__))

typedef struct cstrl_ui_context
{
    void *internal_ui_state;
    void *internal_render_state;
} cstrl_ui_context;

typedef struct cstrl_ui_color
{
    float r, g, b, a;
} cstrl_ui_color;

typedef CSTRL_PACKED_ENUM{CSTRL_HORIZONTAL, CSTRL_VERTICAL} cstrl_ui_layout_direction;

typedef CSTRL_PACKED_ENUM{CSTRL_UI_ALIGN_LEFT, CSTRL_UI_ALIGN_CENTER, CSTRL_UI_ALIGN_RIGHT} cstrl_ui_alignment;

typedef CSTRL_PACKED_ENUM{CSTRL_UI_SIZING_FIT, CSTRL_UI_SIZING_GROW, CSTRL_UI_SIZING_PERCENT,
                          CSTRL_UI_SIZING_FIXED} cstrl_ui_sizing;

typedef CSTRL_PACKED_ENUM{FONT_SIZE_DEFAULT, FONT_SIZE_XSMALL, FONT_SIZE_SMALL,
                          FONT_SIZE_MEDIUM,  FONT_SIZE_LARGE,  FONT_SIZE_XLARGE} cstrl_ui_font_size;

typedef struct cstrl_ui_padding
{
    unsigned short left;
    unsigned short right;
    unsigned short top;
    unsigned short bottom;
} cstrl_ui_padding;

typedef struct cstrl_ui_border_size
{
    unsigned short left;
    unsigned short right;
    unsigned short top;
    unsigned short bottom;
    unsigned short between_children;
} cstrl_ui_border_size;

typedef struct cstrl_ui_border
{
    cstrl_ui_border_size size;
    cstrl_ui_color color;
    cstrl_ui_color gap_color;
    unsigned short gap_size;
} cstrl_ui_border;

typedef struct cstrl_ui_corner_radius
{
    float top_left;
    float top_right;
    float bottom_left;
    float bottom_right;
} cstrl_ui_corner_radius;

typedef struct cstrl_ui_layout
{
    cstrl_ui_border border;
    cstrl_ui_color color;
    cstrl_ui_color font_color;
    cstrl_ui_corner_radius corner_radius;
    cstrl_ui_sizing sizing;
    cstrl_ui_padding padding;
    unsigned short child_gap;
    cstrl_ui_alignment child_alignment;
    cstrl_ui_layout_direction layout_direction;
    cstrl_ui_font_size font_size;
    struct cstrl_ui_layout *layout_hot;
    struct cstrl_ui_layout *layout_active;
} cstrl_ui_layout;

typedef struct cstrl_ui_element
{
    cstrl_ui_layout *layout;
    string text;
    // da_int child_indices;
    int id;
    int index;
    int parent_index;
    int order_priority;
    int x_start, x_end;
    int y_start, y_end;
    bool is_static;
} cstrl_ui_element;

CSTRL_API void cstrl_ui_init(cstrl_ui_context *context, cstrl_platform_state *platform_state);

CSTRL_API void cstrl_ui_begin(cstrl_ui_context *context);

CSTRL_API void cstrl_ui_end(cstrl_ui_context *context);

CSTRL_API void cstrl_ui_shutdown(cstrl_ui_context *context);

CSTRL_API bool cstrl_ui_region_hit(int test_x, int test_y, int object_x, int object_y, int object_width,
                                   int object_height);

CSTRL_API float cstrl_ui_text_width(cstrl_ui_context *context, const char *text, int text_length, float scale,
                                    cstrl_ui_font_size font_size);

CSTRL_API float cstrl_ui_text_height(cstrl_ui_context *context, const char *text, int text_length, float scale,
                                     cstrl_ui_font_size font_size);

CSTRL_API bool cstrl_ui_container_begin(cstrl_ui_context *context, const char *title, int title_length, int *x, int *y,
                                        int w, int h, int id, bool is_static, bool can_minimize, int order_priority,
                                        cstrl_ui_layout *layout);

CSTRL_API void cstrl_ui_container_end(cstrl_ui_context *context);

CSTRL_API bool cstrl_ui_subcontainer_begin(cstrl_ui_context *context, const char *title, int title_length, int x, int y,
                                           int w, int h, int id, cstrl_ui_layout *layout);

CSTRL_API void cstrl_ui_subcontainer_end(cstrl_ui_context *context);

CSTRL_API bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w,
                               int h, int id, cstrl_ui_layout *layout);

CSTRL_API bool cstrl_ui_text(cstrl_ui_context *context, const char *text, int title_length, int x, int y, int w, int h,
                             int id, cstrl_ui_alignment alignment, cstrl_ui_layout *layout);

CSTRL_API bool cstrl_ui_text_field(cstrl_ui_context *context, const char *placeholder, int placeholder_length, int x,
                                   int y, int w, int h, int id, char *buffer, size_t buffer_size);

CSTRL_API bool cstrl_ui_mouse_locked(cstrl_ui_context *context);

CSTRL_API bool cstrl_ui_mouse_over_ui_element(cstrl_ui_context *context);

CSTRL_API void cstrl_ui_set_viewport(cstrl_ui_context *context, int width, int height);

#endif // CSTRL_UI_H
