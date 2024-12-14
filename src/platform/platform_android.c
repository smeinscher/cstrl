//
// Created by 12105 on 12/12/2024.
//

#include "cstrl/cstrl_platform.h"
#if defined(CSTRL_PLATFORM_ANDROID)
#include "log.c/log.h"
#include "platform_internal.h"

#include <stdlib.h>

bool g_should_exit = false;

bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y, int width,
                         int height)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = platform_state->internal_state;

    platform_state->debug_message = "no message";

    state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (state->display == EGL_NO_DISPLAY)
    {
        log_fatal("eglGetDisplay failed");
        platform_state->debug_message = "eglGetDisplay failed";
        return false;
    }

    if (!eglInitialize(state->display, NULL, NULL))
    {
        log_fatal("eglInitialize failed");
        platform_state->debug_message = "eglInitialize failed";
        return false;
    }

    EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                        EGL_OPENGL_ES3_BIT,
                        EGL_SURFACE_TYPE,
                        EGL_WINDOW_BIT,
                        EGL_DEPTH_SIZE,
                        16,
                        EGL_RED_SIZE,
                        8,
                        EGL_GREEN_SIZE,
                        8,
                        EGL_BLUE_SIZE,
                        8,
                        EGL_NONE};
    EGLint num_configs;
    eglChooseConfig(state->display, attribs, &state->config, 1, &num_configs);
    if (num_configs != 1)
    {
        log_fatal("eglChooseConfig failed");
        return false;
    }

    EGLint format;
    eglGetConfigAttrib(state->display, state->config, EGL_NATIVE_VISUAL_ID, &format);
    state->surface = eglCreateWindowSurface(state->display, state->config, platform_state->app_window, NULL);
    if (state->surface == EGL_NO_SURFACE)
    {
        log_fatal("eglCreateWindowSurface failed");
        platform_state->debug_message = "eglCreateWindowSurface failed";
        return false;
    }

    return true;
}

void cstrl_platform_shutdown(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    eglDestroySurface(state->display, state->surface);
    eglTerminate(state->display);
    free(state);
}

void cstrl_platform_pump_messages(cstrl_platform_state *platform_state)
{
}

double cstrl_platform_get_absolute_time()
{
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return current.tv_sec + current.tv_nsec * 0.000000001;
}

void cstrl_platform_sleep(unsigned long long ms)
{
}

bool cstrl_platform_should_exit()
{
    return g_should_exit;
}

void cstrl_platform_set_should_exit(bool should_exit)
{
    g_should_exit = should_exit;
}

void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor)
{
}

#endif