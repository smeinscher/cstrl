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
    cstrl_framebuffer_size_callback framebuffer_size;
    cstrl_mouse_button_callback mouse_button;
    cstrl_mouse_wheel_callback mouse_wheel;
} user_callbacks;

typedef struct input_state
{
    char mouse_buttons[CSTRL_MOUSE_BUTTON_MAX + 1];
    char keys[CSTRL_KEY_MAX + 1];
    int last_mouse_x;
    int last_mouse_y;
    cstrl_mouse_mode mouse_mode;
    bool cursor_shown;
    cstrl_key most_recent_key_pressed;
} input_state;

// Platform-agnostic struct, common for all platforms
typedef struct internal_state_common
{
    user_callbacks callbacks;
    input_state input;
    void *user_data;
    int window_width;
    int window_height;
    int window_x;
    int window_y;
    int initial_window_width;
    int initial_window_height;
    int initial_window_x;
    int initial_window_y;
    int viewport_width;
    int viewport_height;
    int initial_viewport_width;
    int initial_viewport_height;
} internal_state_common;

#if defined(CSTRL_PLATFORM_WINDOWS)

#include <windows.h>

typedef struct internal_state
{
    internal_state_common state_common;
    HINSTANCE h_instance;
    HWND hwnd;
} internal_state;

typedef struct internal_thread_state
{
    HANDLE thread;
    DWORD thread_id;
} internal_thread_state;

#endif

#if defined(CSTRL_PLATFORM_ANDROID)

#include <GLES/egl.h>

typedef struct internal_state
{
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    internal_state_common state_common;
} internal_state;
#endif

#if defined(CSTRL_PLATFORM_LINUX)

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

#if defined(CSTRL_PLATFORM_EM_WEB)

#include <EGL/egl.h>
#include <GLES2/gl2.h>

typedef struct internal_state
{
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    internal_state_common state_common;
} internal_state;

#endif
#endif // PLATFORM_INTERNAL_H
