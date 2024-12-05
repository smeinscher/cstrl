//
// Created by 12105 on 11/23/2024.
//

#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_types.h"

typedef struct user_callbacks
{
    cstrl_key_callback key;
    cstrl_mouse_position_callback mouse_position;
} user_callbacks;

typedef struct input_state
{
    char mouse_buttons[CSTRL_MOUSE_BUTTON_MAX + 1];
    char keys[CSTRL_KEY_MAX + 1];
    int last_mouse_x;
    int last_mouse_y;
    cstrl_mouse_mode mouse_mode;
    bool cursor_shown;
} input_state;

// Platform-agnostic struct, common for all platforms
typedef struct internal_state_common
{
    user_callbacks callbacks;
    input_state input;
} internal_state_common;

#ifdef CSTRL_PLATFORM_WINDOWS

#include <windows.h>

typedef struct internal_state
{
    HINSTANCE h_instance;
    HWND hwnd;
    internal_state_common state_common;
} internal_state;

#endif

#ifdef CSTRL_PLATFORM_LINUX

#include <X11/Xlib.h>
#include <xcb/xcb.h>

typedef struct internal_state
{
    Display *display;
    Window root_window;
    Window main_window;
    internal_state_common state_common;
} internal_state;

#endif

#endif // PLATFORM_INTERNAL_H
