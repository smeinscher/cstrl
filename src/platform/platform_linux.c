//
// Created by sterling on 11/23/24.
//

#include "glad/glad.h"

#include <cstrl/cstrl_platform.h>

#ifdef CSTRL_PLATFORM_LINUX

#include "log.c/log.h"

#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include "platform_internal.h"

static bool g_should_exit = false;

bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y, int width,
                         int height)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = (internal_state *)platform_state->internal_state;

    state->display = XOpenDisplay(NULL);
    if (!state->display)
    {
        log_fatal("Failed to open display");
        return false;
    }

    state->root_window = XDefaultRootWindow(state->display);

    int attribute_value_mask = CWBackPixel | CWEventMask;
    XSetWindowAttributes attributes = {};
    attributes.background_pixel = 0xff000000;
    attributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ExposureMask;

    state->main_window = XCreateWindow(state->display, state->root_window, x, y, width, height, 0, 0, CopyFromParent,
                                       CopyFromParent, attribute_value_mask, &attributes);
    XFlush(state->display);

    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PMinSize | PMaxSize;
    size_hints->min_width = width;
    size_hints->min_height = height;
    size_hints->max_width = width;
    size_hints->max_height = height;
    XSetWMNormalHints(state->display, state->main_window, size_hints);
    XFree(size_hints);

    XStoreName(state->display, state->main_window, application_name);

    return true;
}

void cstrl_platform_destroy(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    XDestroyWindow(state->display, state->main_window);
    XCloseDisplay(state->display);
}

void cstrl_platform_pump_messages(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    XEvent general_event = {};
    if (!XCheckWindowEvent(state->display, state->main_window, ExposureMask | KeyPressMask | ButtonPressMask,
                           &general_event))
    {
        return;
    }

    switch (general_event.type)
    {
    case KeyPress:
    case KeyRelease: {
        XKeyPressedEvent *event = (XKeyPressedEvent *)&general_event;
        if (event->keycode == XKeysymToKeycode(state->display, XK_Escape))
        {
            g_should_exit = true;
        }
    }
    default:
        break;
    }
}

double cstrl_platform_get_absolute_time()
{
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return current.tv_sec + current.tv_nsec * 0.000000001;
}

void cstrl_platform_sleep(unsigned long long ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
}

bool cstrl_platform_should_exit(cstrl_platform_state *platform_state)
{
    return g_should_exit;
}

#endif