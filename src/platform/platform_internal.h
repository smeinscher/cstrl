//
// Created by 12105 on 11/23/2024.
//

#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H
#include "cstrl/cstrl_defines.h"

#ifdef CSTRL_PLATFORM_WINDOWS

#include <windows.h>

typedef struct internal_state
{
    HINSTANCE h_instance;
    HWND hwnd;
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

} internal_state;

#endif

#endif // PLATFORM_INTERNAL_H
