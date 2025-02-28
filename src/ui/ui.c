//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_assert.h"
#include "cstrl/cstrl_platform.h"
#include "ui_internal.h"
#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#if defined(CSTRL_PLATFORM_ANDROID)
#include <glad/gles3/glad.h>
#else
#include "glad/glad.h"
#endif
#include "log.c/log.h"
#include "stb/stb_image_write.h"
#include "stb/stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct cstrl_ui_elements
{
    int element_count;
    da_int ids;
    // x0, y0, x1, y1
    da_int screen_coords;
    // r, g, b, a
    da_float colors;
    da_string titles;
    da_int parent_index;
    da_int order_priority;
} cstrl_ui_elements;

typedef struct cstrl_ui_internal_state
{
    cstrl_platform_state *platform_state;

    cstrl_ui_elements elements;
    cstrl_ui_elements element_cache;
    da_int element_render_order;

    int dragged_element_id;

    int mouse_x;
    int mouse_y;
    int prev_mouse_x;
    int prev_mouse_y;
    bool left_mouse_button_down;
    bool left_mouse_button_processed;
    int left_mouse_button_pressed_x;
    int left_mouse_button_pressed_y;

    int hot_item;
    int active_item;

    cstrl_key key_pressed;
    bool key_press_processed;

    da_int parent_stack;

} cstrl_ui_internal_state;

CSTRL_API void cstrl_ui_init(cstrl_ui_context *context, cstrl_platform_state *platform_state)
{
    context->internal_render_state = cstrl_ui_renderer_init(platform_state);
    context->internal_ui_state = malloc(sizeof(cstrl_ui_internal_state));
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    ui_state->platform_state = platform_state;

    ui_state->elements.element_count = 0;
    cstrl_da_int_init(&ui_state->elements.ids, 1);
    cstrl_da_int_init(&ui_state->elements.screen_coords, 4);
    cstrl_da_float_init(&ui_state->elements.colors, 4);
    cstrl_da_string_init(&ui_state->elements.titles, 1);
    cstrl_da_int_init(&ui_state->elements.parent_index, 1);
    cstrl_da_int_init(&ui_state->elements.order_priority, 1);

    ui_state->element_cache.element_count = 0;
    cstrl_da_int_init(&ui_state->element_cache.ids, 1);
    cstrl_da_int_init(&ui_state->element_cache.screen_coords, 4);
    cstrl_da_float_init(&ui_state->element_cache.colors, 4);
    cstrl_da_string_init(&ui_state->element_cache.titles, 1);
    cstrl_da_int_init(&ui_state->element_cache.parent_index, 1);
    cstrl_da_int_init(&ui_state->element_cache.order_priority, 1);

    cstrl_da_int_init(&ui_state->element_render_order, 1);

    ui_state->dragged_element_id = -1;

    ui_state->mouse_x = -1;
    ui_state->mouse_y = -1;
    ui_state->prev_mouse_x = -1;
    ui_state->prev_mouse_y = -1;

    ui_state->left_mouse_button_down = false;
    ui_state->left_mouse_button_processed = false;
    ui_state->left_mouse_button_pressed_x = -1;
    ui_state->left_mouse_button_pressed_y = -1;

    ui_state->hot_item = -1;
    ui_state->active_item = -1;

    ui_state->key_pressed = CSTRL_KEY_NONE;
    ui_state->key_press_processed = false;

    cstrl_da_int_init(&ui_state->parent_stack, 1);
}

CSTRL_API void cstrl_ui_begin(cstrl_ui_context *context)
{
    cstrl_ui_renderer_new_frame(context->internal_render_state);

    cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    ui_state->elements.element_count = 0;
    cstrl_da_int_clear(&ui_state->elements.ids);
    cstrl_da_int_clear(&ui_state->elements.screen_coords);
    cstrl_da_float_clear(&ui_state->elements.colors);
    cstrl_da_string_clear(&ui_state->elements.titles);
    cstrl_da_int_clear(&ui_state->elements.parent_index);
    cstrl_da_int_clear(&ui_state->elements.order_priority);
    cstrl_da_int_clear(&ui_state->element_render_order);
    cstrl_da_int_clear(&ui_state->parent_stack);

    ui_state->hot_item = -1;
    ui_state->prev_mouse_x = ui_state->mouse_x;
    ui_state->prev_mouse_y = ui_state->mouse_y;
    cstrl_platform_get_cursor_position(ui_state->platform_state, &ui_state->mouse_x, &ui_state->mouse_y);
    bool previous_left_mouse_button_state = ui_state->left_mouse_button_down;
    ui_state->left_mouse_button_down =
        cstrl_platform_is_mouse_button_down(ui_state->platform_state, CSTRL_MOUSE_BUTTON_LEFT);
    if (!ui_state->left_mouse_button_down)
    {
        ui_state->left_mouse_button_pressed_x = -1;
        ui_state->left_mouse_button_pressed_y = -1;
        ui_state->dragged_element_id = -1;
    }
    else if (!previous_left_mouse_button_state)
    {
        ui_state->active_item = -1;
        ui_state->left_mouse_button_pressed_x = ui_state->mouse_x;
        ui_state->left_mouse_button_pressed_y = ui_state->mouse_y;
    }
    ui_state->key_pressed = cstrl_platform_get_most_recent_key_pressed(ui_state->platform_state);
    if (ui_state->key_pressed == CSTRL_KEY_NONE)
    {
        ui_state->key_press_processed = false;
    }
}

CSTRL_API void cstrl_ui_end(cstrl_ui_context *context)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    cstrl_da_int_clear(&ui_state->element_cache.ids);
    cstrl_da_int_clear(&ui_state->element_cache.screen_coords);
    cstrl_da_float_clear(&ui_state->element_cache.colors);
    cstrl_da_string_clear(&ui_state->element_cache.titles);
    cstrl_da_int_clear(&ui_state->element_cache.parent_index);

    ui_state->key_pressed = CSTRL_KEY_NONE;

    ui_state->element_cache.element_count = ui_state->elements.element_count;

    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        cstrl_da_int_push_back(&ui_state->element_cache.ids, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.screen_coords, 0);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        cstrl_da_float_push_back(&ui_state->element_cache.colors, 0.0f);
        string s;
        cstrl_string_init(&s, 7);
        cstrl_string_push_back(&s, "Nothing", 7);
        cstrl_da_string_push_back(&ui_state->element_cache.titles, s);
        cstrl_string_free(&s);
        cstrl_da_int_push_back(&ui_state->element_cache.parent_index, 0);
        cstrl_da_int_push_back(&ui_state->element_cache.order_priority, 0);
    }

    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        int index = ui_state->element_render_order.array[i];
        int x0 = ui_state->elements.screen_coords.array[index * 4];
        int y0 = ui_state->elements.screen_coords.array[index * 4 + 1];
        int x1 = ui_state->elements.screen_coords.array[index * 4 + 2];
        int y1 = ui_state->elements.screen_coords.array[index * 4 + 3];
        cstrl_ui_renderer_add_rect_position(context->internal_render_state, x0, y0, x1, y1);
        float r = ui_state->elements.colors.array[index * 4];
        float g = ui_state->elements.colors.array[index * 4 + 1];
        float b = ui_state->elements.colors.array[index * 4 + 2];
        float a = ui_state->elements.colors.array[index * 4 + 3];
        cstrl_ui_renderer_add_rect_color(context->internal_render_state, r, g, b, a);

        cstrl_ui_renderer_add_rect_uv(context->internal_render_state, 0.0f, 0.0f, 1.0f, 1.0f);

        ui_state->element_cache.ids.array[index] = ui_state->elements.ids.array[index];
        ui_state->element_cache.screen_coords.array[index * 4] = x0;
        ui_state->element_cache.screen_coords.array[index * 4 + 1] = y0;
        ui_state->element_cache.screen_coords.array[index * 4 + 2] = x1;
        ui_state->element_cache.screen_coords.array[index * 4 + 3] = y1;
        ui_state->element_cache.colors.array[index * 4] = r;
        ui_state->element_cache.colors.array[index * 4 + 1] = g;
        ui_state->element_cache.colors.array[index * 4 + 2] = b;
        ui_state->element_cache.colors.array[index * 4 + 3] = a;
        cstrl_string_free(&ui_state->element_cache.titles.array[index]);
        string cached_s;
        cstrl_string_init(&cached_s, ui_state->elements.titles.array[index].size);
        cstrl_string_push_back(&cached_s, ui_state->elements.titles.array[index].array,
                               ui_state->elements.titles.array[index].size);
        ui_state->element_cache.titles.array[index] = cached_s;
        ui_state->element_cache.parent_index.array[index] = ui_state->elements.parent_index.array[index];
        ui_state->element_cache.order_priority.array[index] = ui_state->elements.order_priority.array[index];

        if (ui_state->elements.titles.array[index].size == 0)
        {
            continue;
        }
        // TODO: font stuff
    }
    cstrl_ui_renderer_draw_rects(context->internal_render_state);

    if (ui_state->left_mouse_button_down)
    {
        ui_state->left_mouse_button_processed = true;
    }
    else
    {
        ui_state->left_mouse_button_processed = false;
    }
}

CSTRL_API void cstrl_ui_shutdown(cstrl_ui_context *context)
{
    cstrl_ui_renderer_shutdown(context->internal_render_state);
    free(context->internal_render_state);
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
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    int width = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        stbtt_packedchar c = cstrl_ui_renderer_get_char_data(context->internal_render_state)[text[i]];
        width += c.xadvance;
    }
    return (float)width * scale;
}

CSTRL_API bool cstrl_ui_container_begin(cstrl_ui_context *context, const char *title, int title_length, int x, int y,
                                        int w, int h, int id, bool is_static, bool can_minimize, int order_priority)
{
    cstrl_ui_internal_state *ui_state = context->internal_ui_state;

    CSTRL_ASSERT(ui_state->parent_stack.size == 0, "CSTRL UI: Cannot have a container within a container");

    int original_w = w;
    int original_h = h;
    int original_order_priority = order_priority;
    if (ui_state->element_cache.element_count > 0 &&
        ui_state->element_cache.element_count > ui_state->elements.element_count)
    {
        x = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4];
        y = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4 + 1];
        w = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4 + 2] - x;
        h = ui_state->element_cache.screen_coords.array[ui_state->elements.element_count * 4 + 3] - y;
        order_priority = ui_state->element_cache.order_priority.array[ui_state->elements.element_count];
    }

    if (!is_static && ui_state->dragged_element_id < 0 &&
        cstrl_ui_region_hit(ui_state->mouse_x, ui_state->mouse_y, x, y, w, h) && ui_state->left_mouse_button_down)
    {
        ui_state->dragged_element_id = id;
        ui_state->active_item = id;
    }
    if (!is_static && ui_state->dragged_element_id == id && ui_state->left_mouse_button_down)
    {
        ui_state->active_item = id;
        x -= ui_state->prev_mouse_x - ui_state->mouse_x;
        y -= ui_state->prev_mouse_y - ui_state->mouse_y;
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

    cstrl_da_int_push_back(&ui_state->elements.order_priority, order_priority);
    bool inserted = false;
    for (int i = 0; i < ui_state->elements.element_count; i++)
    {
        if (order_priority >= ui_state->elements.order_priority.array[i])
        {
            cstrl_da_int_insert(&ui_state->element_render_order, ui_state->elements.element_count, i);
            inserted = true;
            break;
        }
    }
    if (!inserted)
    {
        cstrl_da_int_push_back(&ui_state->element_render_order, ui_state->elements.element_count);
    }

    string s;
    cstrl_string_init(&s, title_length);
    cstrl_string_push_back(&s, title, title_length);
    cstrl_da_string_push_back(&ui_state->elements.titles, s);
    cstrl_string_free(&s);

    cstrl_da_int_push_back(&ui_state->elements.parent_index, -1);
    ui_state->elements.element_count++;

    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, x + w);
    cstrl_da_int_push_back(&ui_state->elements.screen_coords, y + h);

    // TODO: get from config somewhere
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.4f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.4f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.4f);
    cstrl_da_float_push_back(&ui_state->elements.colors, 0.7f);

    return true;
}

CSTRL_API void cstrl_ui_container_end(cstrl_ui_context *context)
{
    // cstrl_ui_internal_state *ui_state = context->internal_ui_state;
    // cstrl_da_int_pop_back(&ui_state->parent_stack);
}

CSTRL_API bool cstrl_ui_button(cstrl_ui_context *context, const char *title, int title_length, int x, int y, int w,
                               int h, int id)
{
    return false;
}

CSTRL_API bool cstrl_ui_text(cstrl_ui_context *context, const char *text, int text_length, int x, int y, int w, int h,
                             int id, cstrl_ui_text_alignment alignment)
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
