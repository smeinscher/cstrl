//
// Created by 12105 on 12/12/2024.
//

#include "opengl_platform.h"

#ifdef CSTRL_PLATFORM_ANDROID

#include "../../platform/platform_internal.h"

#include "log.c/log.h"
#include <glad/gles3/glad.h>

EGLContext g_context;

bool cstrl_opengl_platform_init(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;

    EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    g_context = eglCreateContext(state->display, state->config, NULL, attribs);
    if (g_context == EGL_NO_CONTEXT)
    {
        log_fatal("Failed to create context");
        platform_state->debug_message = "eglCreateContext failed";
        return false;
    }

    if (eglMakeCurrent(state->display, state->surface, state->surface, g_context) == EGL_FALSE)
    {
        log_fatal("eglMakeCurrent failed");
        platform_state->debug_message = "eglMakeCurrent failed";
        return false;
    }

    gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
    int width, height;
    eglQuerySurface(state->display, state->surface, EGL_WIDTH, &width);
    eglQuerySurface(state->display, state->surface, EGL_HEIGHT, &height);
    glViewport(0, 0, width, height);
    state->state_common.window_width = width;
    state->state_common.window_height = height;

    return true;
}

void cstrl_opengl_platform_destroy(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    eglDestroyContext(state->display, g_context);
}

void cstrl_opengl_platform_swap_buffers(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    int width, height;
    eglQuerySurface(state->display, state->surface, EGL_WIDTH, &width);
    eglQuerySurface(state->display, state->surface, EGL_HEIGHT, &height);
    glViewport(0, 0, width, height);

    if (eglSwapBuffers(state->display, state->surface) == EGL_FALSE)
    {
        log_error("eglSwapBuffers failed");
    }
}

#endif