//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_assert.h"
#include "cstrl/cstrl_platform.h"
#include "log.c/log.h"
#include "ui_internal.h"
#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include "stb/stb_image_write.h"
#include "stb/stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct cstrl_ui_elements
{
    da_int render_order;
    cstrl_ui_element *elements;
    size_t elements_size;
    unsigned int element_count;
} cstrl_ui_elements;

typedef struct cstrl_ui_mouse_state
{
    int mouse_x;
    int mouse_y;
    int prev_mouse_x;
    int prev_mouse_y;
    int left_mouse_button_pressed_x;
    int left_mouse_button_pressed_y;
    bool left_mouse_button_down;
    bool left_mouse_button_processed;
} cstrl_ui_mouse_state;

typedef struct cstrl_ui_key_state
{
    cstrl_key key_pressed;
    bool key_press_processed;
} cstrl_ui_key_state;

typedef struct cstrl_ui_internal_state
{
    cstrl_ui_elements elements;
    cstrl_ui_elements elements_cache;
    cstrl_ui_mouse_state mouse_state;
    cstrl_ui_key_state key_state;
    da_int parent_stack;
    cstrl_platform_state *platform_state;
    int dragged_element_id;
    int hot_item;
    int active_item;
} cstrl_ui_internal_state;

CSTRL_API void cstrl_ui_init(cstrl_ui_context *context, cstrl_platform_state *platform_state)
{
    context->internal_render_state = cstrl_ui_renderer_init(platform_state);
    context->internal_ui_state = malloc(sizeof(cstrl_ui_internal_state));
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    ui_state->platform_state = platform_state;

    cstrl_da_int_init(&ui_state->elements.render_order, 1);
    ui_state->elements.elements_size = 0;
    ui_state->elements.element_count = 0;
    ui_state->elements.elements = NULL;

    cstrl_da_int_init(&ui_state->elements_cache.render_order, 1);
    ui_state->elements_cache.elements_size = 0;
    ui_state->elements_cache.element_count = 0;
    ui_state->elements_cache.elements = NULL;

    ui_state->dragged_element_id = -1;

    ui_state->mouse_state.mouse_x = -1;
    ui_state->mouse_state.mouse_y = -1;
    ui_state->mouse_state.prev_mouse_x = -1;
    ui_state->mouse_state.prev_mouse_y = -1;

    ui_state->mouse_state.left_mouse_button_down = false;
    ui_state->mouse_state.left_mouse_button_processed = false;
    ui_state->mouse_state.left_mouse_button_pressed_x = -1;
    ui_state->mouse_state.left_mouse_button_pressed_y = -1;

    ui_state->hot_item = -1;
    ui_state->active_item = -1;

    ui_state->key_state.key_pressed = CSTRL_KEY_NONE;
    ui_state->key_state.key_press_processed = false;

    cstrl_da_int_init(&ui_state->parent_stack, 1);
}

CSTRL_API void cstrl_ui_begin(cstrl_ui_context *context)
{
    cstrl_ui_renderer_new_frame(context->internal_render_state);

    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    ui_state->elements.element_count = 0;
    cstrl_da_int_clear(&ui_state->elements.render_order);
    cstrl_da_int_clear(&ui_state->parent_stack);

    ui_state->hot_item = -1;
    ui_state->mouse_state.prev_mouse_x = ui_state->mouse_state.mouse_x;
    ui_state->mouse_state.prev_mouse_y = ui_state->mouse_state.mouse_y;
    cstrl_platform_get_cursor_position(ui_state->platform_state, &ui_state->mouse_state.mouse_x,
                                       &ui_state->mouse_state.mouse_y);
    bool previous_left_mouse_button_state = ui_state->mouse_state.left_mouse_button_down;
    ui_state->mouse_state.left_mouse_button_down =
        cstrl_platform_is_mouse_button_down(ui_state->platform_state, CSTRL_MOUSE_BUTTON_LEFT);
    if (!ui_state->mouse_state.left_mouse_button_down)
    {
        ui_state->mouse_state.left_mouse_button_pressed_x = -1;
        ui_state->mouse_state.left_mouse_button_pressed_y = -1;
        ui_state->dragged_element_id = -1;
    }
    else if (!previous_left_mouse_button_state)
    {
        ui_state->active_item = -1;
        ui_state->mouse_state.left_mouse_button_pressed_x = ui_state->mouse_state.mouse_x;
        ui_state->mouse_state.left_mouse_button_pressed_y = ui_state->mouse_state.mouse_y;
    }
    ui_state->key_state.key_pressed = cstrl_platform_get_most_recent_key_pressed(ui_state->platform_state);
    if (ui_state->key_state.key_pressed == CSTRL_KEY_NONE)
    {
        ui_state->key_state.key_press_processed = false;
    }
}

CSTRL_API void cstrl_ui_end(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    cstrl_da_int_clear(&ui_state->elements_cache.render_order);
    for (int i = 0; i < ui_state->elements.render_order.size; i++)
    {
        cstrl_da_int_push_back(&ui_state->elements_cache.render_order, ui_state->elements.render_order.array[i]);
    }
    for (int i = 0; i < ui_state->elements_cache.elements_size; i++)
    {
        cstrl_string_free(&ui_state->elements_cache.elements[i].text);
    }
    free(ui_state->elements_cache.elements);
    ui_state->elements_cache.element_count = ui_state->elements.element_count;
    ui_state->elements_cache.elements_size = ui_state->elements.element_count;
    ui_state->elements_cache.elements = malloc(ui_state->elements.element_count * sizeof(cstrl_ui_element));
    CSTRL_ASSERT(ui_state->elements_cache.elements, "Failed to realloc ui element cache");
    ui_state->key_state.key_pressed = CSTRL_KEY_NONE;

    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        int index = ui_state->elements.render_order.array[i];
        int x0 = ui_state->elements.elements[index].x_start;
        int y0 = ui_state->elements.elements[index].y_start;
        int x1 = ui_state->elements.elements[index].x_end;
        int y1 = ui_state->elements.elements[index].y_end;
        cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y0, x1, y1);
        cstrl_ui_layout *layout = ui_state->elements.elements[index].layout;
        if (layout != NULL)
        {
            short bs_top = layout->border.size.top;
            short bs_bottom = layout->border.size.bottom;
            short bs_left = layout->border.size.left;
            short bs_right = layout->border.size.right;
            short gap = layout->border.gap_size;
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y0, x1, y0 + bs_top);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y1 - bs_bottom, x1, y1);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y0, x0 + bs_left, y1);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x1 - bs_right, y0, x1, y1);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0 + bs_top, y0 + bs_top, x1 - bs_top,
                                                y0 + bs_top + gap);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0 + bs_bottom, y1 - bs_bottom - gap,
                                                x1 - bs_bottom, y1 - bs_bottom);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0 + bs_left, y0 + bs_left,
                                                x0 + bs_left + gap, y1 - bs_left);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x1 - bs_right - gap, y0 + bs_right,
                                                x1 - bs_right, y1 - bs_right);
            float r = layout->color.r;
            float g = layout->color.g;
            float b = layout->color.b;
            float a = layout->color.a;
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, r, g, b, a);
            float br = layout->border.color.r;
            float bg = layout->border.color.g;
            float bb = layout->border.color.b;
            float ba = layout->border.color.a;
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, br, bg, bb, ba);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, br, bg, bb, ba);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, br, bg, bb, ba);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, br, bg, bb, ba);
            float gr = layout->border.gap_color.r;
            float gg = layout->border.gap_color.g;
            float gb = layout->border.gap_color.b;
            float ga = layout->border.gap_color.a;
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, gr, gg, gb, ga);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, gr, gg, gb, ga);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, gr, gg, gb, ga);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, gr, gg, gb, ga);
        }
        else
        {
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, 0.4f, 0.4f, 0.4f, 0.7f);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y0, x1, y0 + 2);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y1 - 2, x1, y1);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y0, x0 + 2, y1);
            cstrl_ui_renderer_add_rect_position(context->internal_render_state, x1 - 2, y0, x1, y1);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, 0.0f, 0.0f, 0.0f, 1.0f);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, 0.0f, 0.0f, 0.0f, 1.0f);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, 0.0f, 0.0f, 0.0f, 1.0f);
            cstrl_ui_renderer_add_rect_color(context->internal_render_state, 0.0f, 0.0f, 0.0f, 1.0f);
        }

        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);
        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);

        ui_state->elements_cache.elements[index].index = ui_state->elements.elements[index].index;
        ui_state->elements_cache.elements[index].x_start = x0;
        ui_state->elements_cache.elements[index].y_start = y0;
        ui_state->elements_cache.elements[index].x_end = x1;
        ui_state->elements_cache.elements[index].y_end = y1;
        ui_state->elements_cache.elements[index].parent_index = ui_state->elements.elements[index].parent_index;
        ui_state->elements_cache.elements[index].order_priority = ui_state->elements.elements[index].order_priority;

        if (ui_state->elements.elements[index].text.size == 0)
        {
            continue;
        }
        cstrl_string_init(&ui_state->elements_cache.elements[index].text, ui_state->elements.elements[index].text.size);
        cstrl_string_push_back(&ui_state->elements_cache.elements[index].text,
                               ui_state->elements.elements[index].text.array,
                               ui_state->elements.elements[index].text.size);

        cstrl_ui_renderer_add_font(context->internal_render_state, ui_state->elements.elements[index].text.array, 0,
                                   ui_state->elements.elements[index].text.size, x0 + 10, y0 + 20, layout->font_color.r,
                                   layout->font_color.r, layout->font_color.b, layout->font_color.a);
        cstrl_string_free(&ui_state->elements.elements[index].text);
    }
    cstrl_ui_renderer_draw_rects(context->internal_render_state);
    cstrl_ui_renderer_draw_font(context->internal_render_state);
    if (ui_state->mouse_state.left_mouse_button_down)
    {
        ui_state->mouse_state.left_mouse_button_processed = true;
    }
    else
    {
        ui_state->mouse_state.left_mouse_button_processed = false;
    }
}

CSTRL_API void cstrl_ui_shutdown(cstrl_ui_context *context)
{
    cstrl_ui_renderer_shutdown(context->internal_render_state);
    free(context->internal_render_state);

    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    cstrl_da_int_free(&ui_state->elements.render_order);
    for (int i = 0; i < ui_state->elements.elements_size; i++)
    {
        // cstrl_string_free(&ui_state->elements.elements[i].text);
        // cstrl_da_int_free(&ui_state->elements.elements[i].child_indices);
    }
    free(ui_state->elements.elements);
    cstrl_da_int_free(&ui_state->elements_cache.render_order);
    for (int i = 0; i < ui_state->elements_cache.elements_size; i++)
    {
        // cstrl_string_free(&ui_state->elements_cache.elements[i].text);
        // cstrl_da_int_free(&ui_state->elements_cache.elements[i].child_indices);
    }
    free(ui_state->elements_cache.elements);

    cstrl_da_int_free(&ui_state->parent_stack);

    free(ui_state);
}

CSTRL_API bool cstrl_ui_region_hit(int test_x, int test_y, int object_x, int object_y, int object_width,
                                   int object_height)
{
    if (test_x < object_x || test_y < object_y || test_x > object_x + object_width || test_y > object_y + object_height)
    {
        return false;
    }
    return true;
}

CSTRL_API float cstrl_ui_text_width(cstrl_ui_context *context, const char *text, float scale)
{
    // cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    int width = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        stbtt_packedchar c = cstrl_ui_renderer_get_char_data(context->internal_render_state)[(int)text[i]];
        width += c.xadvance;
    }
    return (float)width * scale;
}

CSTRL_API bool cstrl_ui_container_begin(cstrl_ui_context *context, const char *title, int title_length, int x, int y,
                                        int w, int h, int id, bool is_static, bool can_minimize, int order_priority,
                                        cstrl_ui_layout *layout)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    // CSTRL_ASSERT(ui_state->parent_stack.size == 0, "CSTRL UI: Cannot have a container within a container");

    if (ui_state->elements.element_count >= ui_state->elements.elements_size)
    {
        ui_state->elements.elements_size =
            ui_state->elements.elements_size != 0 ? ui_state->elements.elements_size * 2 : 1;
        cstrl_ui_element *tmp =
            realloc(ui_state->elements.elements, ui_state->elements.elements_size * sizeof(cstrl_ui_element));
        CSTRL_ASSERT(tmp, "Failed to realloc ui elements");
        ui_state->elements.elements = tmp;
    }
    int index = ui_state->elements.element_count;
    ui_state->elements.elements[index] = (cstrl_ui_element){0};
    int original_order_priority = order_priority;
    if (ui_state->elements_cache.element_count > 0 && ui_state->elements_cache.element_count > index)
    {
        x = ui_state->elements_cache.elements[index].x_start;
        y = ui_state->elements_cache.elements[index].y_start;
        w = ui_state->elements_cache.elements[index].x_end - x;
        h = ui_state->elements_cache.elements[index].y_end - y;
        order_priority = ui_state->elements_cache.elements[index].order_priority;
    }

    if (!is_static && ui_state->dragged_element_id < 0 &&
        cstrl_ui_region_hit(ui_state->mouse_state.mouse_x, ui_state->mouse_state.mouse_y, x, y, w, h) &&
        ui_state->mouse_state.left_mouse_button_down)
    {
        ui_state->dragged_element_id = id;
        ui_state->active_item = id;
    }
    if (!is_static && ui_state->dragged_element_id == id && ui_state->mouse_state.left_mouse_button_down)
    {
        ui_state->active_item = id;
        x -= ui_state->mouse_state.prev_mouse_x - ui_state->mouse_state.mouse_x;
        y -= ui_state->mouse_state.prev_mouse_y - ui_state->mouse_state.mouse_y;
    }

    if (ui_state->active_item == id)
    {
        order_priority = -1;
    }
    else
    {
        order_priority = original_order_priority;
    }

    int width, height;
    cstrl_platform_get_window_size(ui_state->platform_state, &width, &height);
    if (width != 0 && height != 0)
    {
        if (x < 0)
        {
            x = 0;
        }
        else if (x > width - w)
        {
            x = width - w;
        }
        if (y < 0)
        {
            y = 0;
        }
        else if (y > height - h)
        {
            y = height - h;
        }
    }

    cstrl_da_int_push_back(&ui_state->elements.render_order, index);
    /*
    cstrl_da_int_push_back(&ui_state->elements.render_order, order_priority);
    bool inserted = false;
    for (int i = 0; i < index; i++)
    {
        if (order_priority >= ui_state->elements.elements[i].order_priority)
        {
            cstrl_da_int_insert(&ui_state->elements.render_order, index, i);
            inserted = true;
            break;
        }
    }
    if (!inserted)
    {
        cstrl_da_int_push_back(&ui_state->elements.render_order, index);
    }
    */

    cstrl_string_init(&ui_state->elements.elements[index].text, title_length);
    cstrl_string_push_back(&ui_state->elements.elements[index].text, title, title_length);

    ui_state->elements.elements[index].parent_index = -1;
    ui_state->elements.element_count++;

    cstrl_da_int_push_back(&ui_state->parent_stack, index);
    ui_state->elements.elements[index].index = index;
    ui_state->elements.elements[index].id = id;

    ui_state->elements.elements[index].x_start = x;
    ui_state->elements.elements[index].y_start = y;
    ui_state->elements.elements[index].x_end = x + w;
    ui_state->elements.elements[index].y_end = y + h;

    ui_state->elements.elements[index].layout = layout;

    return true;
}

CSTRL_API void cstrl_ui_container_end(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    cstrl_da_int_pop_back(&ui_state->parent_stack);
}

CSTRL_API bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w,
                               int h, int id)
{
    return false;
}

CSTRL_API bool cstrl_ui_text(cstrl_ui_context *context, const char *text, int text_length, int x, int y, int w, int h,
                             int id, cstrl_ui_alignment alignment)
{
    return false;
}

CSTRL_API bool cstrl_ui_text_field(cstrl_ui_context *context, const char *placeholder, int placeholder_length, int x,
                                   int y, int w, int h, int id, char *buffer, size_t buffer_size)
{
    return false;
}

CSTRL_API bool cstrl_ui_mouse_locked(cstrl_ui_context *context)
{
    return false;
}
