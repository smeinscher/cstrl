#include "cstrl/cstrl_platform.h"
#include "platform_internal.h"

#ifdef CSTRL_PLATFORM_EM_WEB

#include <emscripten.h>
#include <emscripten/html5.h>

#include "cstrl/cstrl_assert.h"
#include "log.c/log.h"

bool g_should_exit = false;

static int convert_string_to_cstrl_key(const char *key_string)
{
    char key = key_string[0];

    if (key >= 'a' && key <= 'z')
    {
        key -= 32;
    }
    else if (key >= '0' && key <= '9')
    {
        key -= 18;
    }

    return key;
}

static bool key_callback(int event_type, const EmscriptenKeyboardEvent *e, void *user_data)
{
    cstrl_platform_state *state = user_data;
    internal_state *internal_state = state->internal_state;
    if (internal_state->state_common.callbacks.key == NULL)
    {
        log_warn("No key callback");
        return 0;
    }
    int key = convert_string_to_cstrl_key(e->key);

    int action;
    switch (event_type)
    {
    case EMSCRIPTEN_EVENT_KEYDOWN:
    case EMSCRIPTEN_EVENT_KEYPRESS:
        action = CSTRL_ACTION_PRESS;
        break;
    case EMSCRIPTEN_EVENT_KEYUP:
        action = CSTRL_ACTION_RELEASE;
        break;
    default:
        action = CSTRL_ACTION_NONE;
        break;
    }
    // TODO: investigate scancode, implement mods
    internal_state->state_common.callbacks.key(state, key, 0, action, 0);
    return 1;
}

CSTRL_API bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y,
                                   int width, int height)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = platform_state->internal_state;

    state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (state->display == EGL_NO_DISPLAY)
    {
        log_fatal("eglGetDisplay failed");
        return false;
    }

    if (!eglInitialize(state->display, NULL, NULL))
    {
        log_fatal("eglInitialize failed");
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

    // EGLint format;
    // eglGetConfigAttrib(state->display, state->config, EGL_NATIVE_VISUAL_ID, &format);
    state->surface = eglCreateWindowSurface(state->display, state->config, 0, NULL);
    if (state->surface == EGL_NO_SURFACE)
    {
        log_fatal("eglCreateWindowSurface failed");
        return false;
    }

    state->state_common.callbacks.key = NULL;
    state->state_common.callbacks.mouse_position = NULL;
    state->state_common.callbacks.framebuffer_size = NULL;
    state->state_common.callbacks.mouse_button = NULL;
    state->state_common.callbacks.mouse_wheel = NULL;

    state->state_common.input.mouse_mode = CSTRL_MOUSE_NORMAL;

    state->state_common.window_width = width;
    state->state_common.window_height = height;

    EMSCRIPTEN_RESULT result =
        emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, platform_state, 1, key_callback);
    CSTRL_ASSERT(result == EMSCRIPTEN_RESULT_SUCCESS, "Failed to set keypress callback");
    result = emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, platform_state, 1, key_callback);
    CSTRL_ASSERT(result == EMSCRIPTEN_RESULT_SUCCESS, "Failed to set keypress callback");
    result = emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, platform_state, 1, key_callback);
    CSTRL_ASSERT(result == EMSCRIPTEN_RESULT_SUCCESS, "Failed to set keypress callback");
    return true;
}

CSTRL_API void cstrl_platform_shutdown(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    eglDestroySurface(state->display, state->surface);
    eglTerminate(state->display);
    free(state);
}

CSTRL_API void cstrl_platform_pump_messages(cstrl_platform_state *platform_state)
{
}

CSTRL_API double cstrl_platform_get_absolute_time()
{
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return current.tv_sec + current.tv_nsec * 0.000000001;
}

CSTRL_API void cstrl_platform_sleep(unsigned long long ms)
{
}

CSTRL_API bool cstrl_platform_should_exit()
{
    return g_should_exit;
}

CSTRL_API void cstrl_platform_set_should_exit(bool should_exit)
{
    g_should_exit = should_exit;
}

CSTRL_API void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor)
{
}

#endif
