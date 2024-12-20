//
// Created by 12105 on 11/27/2024.
//

#include "cstrl/cstrl_platform.h"
#include "log.c/log.h"
#include "platform_internal.h"

CSTRL_API void cstrl_platform_get_window_size(cstrl_platform_state *platform_state, int *width, int *height)
{
    internal_state *state = platform_state->internal_state;
    *width = state->state_common.window_width;
    *height = state->state_common.window_height;
}

CSTRL_API void cstrl_platform_get_cursor_position(cstrl_platform_state *platform_state, int *x, int *y)
{
    internal_state *state = platform_state->internal_state;
    *x = state->state_common.input.last_mouse_x;
    *y = state->state_common.input.last_mouse_y;
}

CSTRL_API bool cstrl_platform_is_mouse_button_down(cstrl_platform_state *platform_state, cstrl_mouse_button button)
{
    internal_state *state = platform_state->internal_state;
    if (button >= CSTRL_MOUSE_BUTTON_MAX)
    {
        log_warn("Button %d is out of range", button);
        return false;
    }
    return state->state_common.input.mouse_buttons[button] == CSTRL_ACTION_PRESS;
}

CSTRL_API cstrl_key cstrl_platform_get_most_recent_key_pressed(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    return state->state_common.input.most_recent_key_pressed;
}

CSTRL_API cstrl_mouse_mode cstrl_platform_get_mouse_mode(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    return state->state_common.input.mouse_mode;
}

CSTRL_API void cstrl_platform_set_mouse_mode(cstrl_platform_state *platform_state, cstrl_mouse_mode mode)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.input.mouse_mode = mode;
}

CSTRL_API void cstrl_platform_set_key_callback(cstrl_platform_state *platform_state, cstrl_key_callback key_callback)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.callbacks.key = key_callback;
}

CSTRL_API void cstrl_platform_set_mouse_position_callback(cstrl_platform_state *platform_state,
                                                          cstrl_mouse_position_callback mouse_position_callback)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.callbacks.mouse_position = mouse_position_callback;
}

CSTRL_API void cstrl_platform_set_framebuffer_size_callback(cstrl_platform_state *platform_state,
                                                            cstrl_framebuffer_size_callback framebuffer_size_callback)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.callbacks.framebuffer_size = framebuffer_size_callback;
}
