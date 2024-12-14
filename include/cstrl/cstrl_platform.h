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

bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y, int width,
                         int height);

void cstrl_platform_shutdown(cstrl_platform_state *platform_state);

void cstrl_platform_pump_messages(cstrl_platform_state *platform_state);

double cstrl_platform_get_absolute_time();

void cstrl_platform_sleep(unsigned long long ms);

bool cstrl_platform_should_exit();

void cstrl_platform_set_should_exit(bool should_exit);

void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor);

/*
 *
 *  Common
 *
 */
void cstrl_platform_get_window_size(cstrl_platform_state *platform_state, int *width, int *height);

void cstrl_platform_get_cursor_position(cstrl_platform_state *platform_state, int *x, int *y);

bool cstrl_platform_is_mouse_button_down(cstrl_platform_state *platform_state, cstrl_mouse_button button);

cstrl_key cstrl_platform_get_most_recent_key_pressed(cstrl_platform_state *platform_state);

cstrl_mouse_mode cstrl_platform_get_mouse_mode(cstrl_platform_state *platform_state);

void cstrl_platform_set_mouse_mode(cstrl_platform_state *platform_state, cstrl_mouse_mode mode);

void cstrl_platform_set_key_callback(cstrl_platform_state *platform_state, cstrl_key_callback key_callback);

void cstrl_platform_set_mouse_position_callback(cstrl_platform_state *platform_state,
                                                cstrl_mouse_position_callback mouse_position_callback);

#endif // CSTRL_PLATFORM_H
