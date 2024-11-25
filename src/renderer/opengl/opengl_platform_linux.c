//
// Created by sterling on 11/23/24.
//

#include <cstrl/cstrl_defines.h>

#ifdef CSTRL_PLATFORM_LINUX

#include "../../platform/platform_internal.h"
#include "cstrl_opengl_platform.h"
#include "glad/glad.h"
#include "log.c/log.h"
#include <GL/glx.h>

GLXContext gl_context;

bool cstrl_opengl_platform_init(cstrl_platform_state *platform_state)
{
    internal_state_new *state = (internal_state_new *)platform_state->internal_state;
    int attribute_list[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    XVisualInfo *visual_info = glXChooseVisual(state->display, 0, attribute_list);
    if (visual_info == NULL)
    {
        log_fatal("Unable to choose visual");
        return false;
    }
    gl_context = glXCreateContext(state->display, visual_info, NULL, GL_TRUE);
    if (gl_context == NULL)
    {
        log_fatal("Unable to create GL context");
        return false;
    }

    glXMakeCurrent(state->display, state->main_window, gl_context);
    gladLoadGL();
    glViewport(0, 0, 800, 600);

    XMapWindow(state->display, state->main_window);
    return true;
}

void cstrl_opengl_platform_destroy(cstrl_platform_state *platform_state)
{
    internal_state_new *state = (internal_state_new *)platform_state->internal_state;
    glXDestroyContext(state->display, gl_context);
}

#endif
