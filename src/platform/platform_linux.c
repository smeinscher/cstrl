//
// Created by sterling on 11/23/24.
//

#include <cstrl/cstrl_platform.h>

#if defined(CSTRL_PLATFORM_LINUX)

#include "cstrl/cstrl_types.h"
#include "log.c/log.h"

#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include "platform_internal.h"

static bool g_should_exit = false;

static short x11_key_to_cstrl_key[192];

static Atom wm_delete_message;

void x11_key_to_cstrl_key_init(internal_state *state)
{
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Control_L)] = CSTRL_KEY_LEFT_CONTROL;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Control_R)] = CSTRL_KEY_RIGHT_CONTROL;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Shift_L)] = CSTRL_KEY_LEFT_SHIFT;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Shift_R)] = CSTRL_KEY_RIGHT_SHIFT;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_BackSpace)] = CSTRL_KEY_BACKSPACE;

    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Tab)] = CSTRL_KEY_TAB;

    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Escape)] = CSTRL_KEY_ESCAPE;

    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_0)] = CSTRL_KEY_0;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_1)] = CSTRL_KEY_1;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_2)] = CSTRL_KEY_2;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_3)] = CSTRL_KEY_3;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_4)] = CSTRL_KEY_4;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_5)] = CSTRL_KEY_5;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_6)] = CSTRL_KEY_6;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_7)] = CSTRL_KEY_7;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_8)] = CSTRL_KEY_8;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_9)] = CSTRL_KEY_9;

    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_period)] = CSTRL_KEY_PERIOD;

    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_A)] = CSTRL_KEY_A;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_B)] = CSTRL_KEY_B;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_C)] = CSTRL_KEY_C;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_D)] = CSTRL_KEY_D;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_E)] = CSTRL_KEY_E;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_F)] = CSTRL_KEY_F;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_G)] = CSTRL_KEY_G;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_H)] = CSTRL_KEY_H;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_I)] = CSTRL_KEY_I;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_J)] = CSTRL_KEY_J;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_K)] = CSTRL_KEY_K;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_L)] = CSTRL_KEY_L;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_M)] = CSTRL_KEY_M;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_N)] = CSTRL_KEY_N;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_O)] = CSTRL_KEY_O;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_P)] = CSTRL_KEY_P;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Q)] = CSTRL_KEY_Q;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_R)] = CSTRL_KEY_R;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_S)] = CSTRL_KEY_S;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_T)] = CSTRL_KEY_T;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_U)] = CSTRL_KEY_U;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_V)] = CSTRL_KEY_V;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_W)] = CSTRL_KEY_W;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_X)] = CSTRL_KEY_X;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Y)] = CSTRL_KEY_Y;
    x11_key_to_cstrl_key[XKeysymToKeycode(state->display, XK_Z)] = CSTRL_KEY_Z;
}

bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y, int width,
                         int height)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = platform_state->internal_state;

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
    attributes.event_mask = StructureNotifyMask | ExposureMask | ExposureMask | KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    state->main_window = XCreateWindow(state->display, state->root_window, x, y, width, height, 0, 0, CopyFromParent,
                                       CopyFromParent, attribute_value_mask, &attributes);
    XFlush(state->display);

    wm_delete_message = XInternAtom(state->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(state->display, state->main_window, &wm_delete_message, 1);

    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PMinSize | PMaxSize;
    size_hints->min_width = width;
    size_hints->min_height = height;
    size_hints->max_width = width;
    size_hints->max_height = height;
    XSetWMNormalHints(state->display, state->main_window, size_hints);
    XFree(size_hints);

    XStoreName(state->display, state->main_window, application_name);

    x11_key_to_cstrl_key_init(state);

    XWarpPointer(state->display, state->main_window, state->main_window, 0, 0, width, height, width / 2, height / 2);

    state->state_common.window_width = width;
    state->state_common.window_height = height;

    state->state_common.callbacks.key = NULL;
    state->state_common.callbacks.mouse_position = NULL;

    memset(state->state_common.input.mouse_buttons, 0, CSTRL_MOUSE_BUTTON_MAX + 1);
    memset(state->state_common.input.keys, 0, CSTRL_KEY_MAX + 1);
    state->state_common.input.last_mouse_x = -1;
    state->state_common.input.last_mouse_y = -1;
    state->state_common.input.mouse_mode = CSTRL_MOUSE_NORMAL;
    state->state_common.input.cursor_shown = true;
    state->state_common.input.most_recent_key_pressed = CSTRL_KEY_NONE;

    XMapWindow(state->display, state->main_window);
    return true;
}

void cstrl_platform_shutdown(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    XDestroyWindow(state->display, state->main_window);
    XCloseDisplay(state->display);
    free(state);
}

void cstrl_platform_pump_messages(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    XPending(state->display);
    // XEvent general_event = {};
    /* if (!XCheckWindowEvent(state->display, state->main_window,
                           ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
                               PointerMotionMask | PointerMotionHintMask,
                           &general_event))
    {
        return;
    }
    */
    while (QLength(state->display))
    {
        XEvent general_event;
        XNextEvent(state->display, &general_event);
        switch (general_event.type)
        {
        case KeyPress:
        case KeyRelease: {
            XKeyPressedEvent *event = (XKeyPressedEvent *)&general_event;
            const int action = event->type == KeyPress ? CSTRL_ACTION_PRESS : CSTRL_ACTION_RELEASE;

            if (state->state_common.callbacks.key != NULL)
            {
                state->state_common.callbacks.key(platform_state, x11_key_to_cstrl_key[event->keycode], event->keycode,
                                                  action, 0);
            }
            if (action == CSTRL_ACTION_PRESS)
            {
                state->state_common.input.most_recent_key_pressed = x11_key_to_cstrl_key[event->keycode];
            }
            else
            {
                state->state_common.input.most_recent_key_pressed = CSTRL_KEY_NONE;
            }
            break;
        }
        case ButtonPress:
        case ButtonRelease: {
            XButtonPressedEvent *event = (XButtonPressedEvent *)&general_event;
            cstrl_mouse_button button;
            if (event->button == Button1)
            {
                button = CSTRL_MOUSE_BUTTON_LEFT;
            }
            else if (event->button == Button2)
            {
                button = CSTRL_MOUSE_BUTTON_MIDDLE;
            }
            else if (event->button == Button3)
            {
                button = CSTRL_MOUSE_BUTTON_RIGHT;
            }
            else
            {
                button = CSTRL_MOUSE_BUTTON_UNKNOWN;
            }
            cstrl_action action = event->type == ButtonPress ? CSTRL_ACTION_PRESS : CSTRL_ACTION_RELEASE;

            state->state_common.input.mouse_buttons[button] = action;
            break;
        }
        case MotionNotify: {
            XMotionEvent *event = (XMotionEvent *)&general_event;
            if (state->state_common.callbacks.mouse_position != NULL)
            {
                state->state_common.callbacks.mouse_position(platform_state, event->x, event->y);
            }
            state->state_common.input.last_mouse_x = event->x;
            state->state_common.input.last_mouse_y = event->y;
            break;
        }
        case ClientMessage: {
            if (general_event.xclient.data.l[0] == wm_delete_message)
            {
                g_should_exit = true;
            }
        }
        default:
            break;
        }
    }
    if (state->state_common.input.mouse_mode == CSTRL_MOUSE_DISABLED && state->state_common.input.last_mouse_x != 400 &&
        state->state_common.input.last_mouse_y != 300)
    {
        XWarpPointer(state->display, None, state->main_window, 0, 0, 0, 0, 400, 300);
    }
    XFlush(state->display);
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

void cstrl_platform_set_should_exit(bool should_exit)
{
    g_should_exit = should_exit;
}

void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.input.cursor_shown = show_cursor;

    show_cursor ? XFixesShowCursor(state->display, state->main_window)
                : XFixesHideCursor(state->display, state->main_window);
}

#endif