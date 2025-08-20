//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_PLATFORM_H
#define CSTRL_PLATFORM_H

#include "cstrl_defines.h"
#include "cstrl_types.h"

#include <stdbool.h>

typedef struct cstrl_platform_state
{
    void *internal_state;
#if defined(CSTRL_PLATFORM_ANDROID)
    void *app_window;
    const char *debug_message;
#endif
} cstrl_platform_state;

typedef void (*cstrl_key_callback)(cstrl_platform_state *state, int key, int scancode, int action, int mods);

typedef void (*cstrl_mouse_position_callback)(cstrl_platform_state *state, int x, int y);

typedef void (*cstrl_framebuffer_size_callback)(cstrl_platform_state *state, int width, int height);

typedef void (*cstrl_mouse_button_callback)(cstrl_platform_state *state, int button, int action, int mods);

typedef void (*cstrl_mouse_wheel_callback)(cstrl_platform_state *state, int delta_x, int delta_y, int keys_down);

CSTRL_API bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y,
                                   int width, int height, bool fullscreen);

CSTRL_API void cstrl_platform_shutdown(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_platform_pump_messages(cstrl_platform_state *platform_state);

CSTRL_API double cstrl_platform_get_absolute_time();

CSTRL_API void cstrl_platform_sleep(unsigned long long ms);

CSTRL_API bool cstrl_platform_should_exit();

CSTRL_API void cstrl_platform_set_should_exit(bool should_exit);

CSTRL_API void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor);

CSTRL_API void cstrl_platform_get_screen_resolution(int *width, int *height);

CSTRL_API void cstrl_platform_set_fullscreen(cstrl_platform_state *platform_state, bool fullscreen);

/*
 *
 *  Common
 *
 */

CSTRL_API void cstrl_platform_get_window_size(cstrl_platform_state *platform_state, int *width, int *height);

CSTRL_API void cstrl_platform_get_viewport_size(cstrl_platform_state *platform_state, int *width, int *height);

CSTRL_API void cstrl_platform_get_cursor_position(cstrl_platform_state *platform_state, int *x, int *y);

CSTRL_API bool cstrl_platform_is_mouse_button_down(cstrl_platform_state *platform_state, cstrl_mouse_button button);

CSTRL_API cstrl_key cstrl_platform_get_most_recent_key_pressed(cstrl_platform_state *platform_state);

CSTRL_API cstrl_mouse_mode cstrl_platform_get_mouse_mode(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_platform_set_mouse_mode(cstrl_platform_state *platform_state, cstrl_mouse_mode mode);

CSTRL_API void cstrl_platform_set_key_callback(cstrl_platform_state *platform_state, cstrl_key_callback key_callback);

CSTRL_API void cstrl_platform_set_mouse_position_callback(cstrl_platform_state *platform_state,
                                                          cstrl_mouse_position_callback mouse_position_callback);

CSTRL_API void cstrl_platform_set_framebuffer_size_callback(cstrl_platform_state *platform_state,
                                                            cstrl_framebuffer_size_callback framebuffer_size_callback);

CSTRL_API void cstrl_platform_set_mouse_button_callback(cstrl_platform_state *platform_state,
                                                        cstrl_mouse_button_callback mouse_button_callback);

CSTRL_API void cstrl_platform_set_mouse_wheel_callback(cstrl_platform_state *platform_state,
                                                       cstrl_mouse_wheel_callback mouse_wheel_callback);

CSTRL_API void *cstrl_platform_get_user_data(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_platform_set_user_data(cstrl_platform_state *platform_state, void *user_data);

CSTRL_API void cstrl_platform_free_user_data(cstrl_platform_state *platform_state);

#endif // CSTRL_PLATFORM_H
