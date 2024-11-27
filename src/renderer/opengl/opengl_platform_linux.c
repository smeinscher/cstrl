//
// Created by sterling on 11/23/24.
//

#include <cstrl/cstrl_defines.h>

#ifdef CSTRL_PLATFORM_LINUX

#include "../../platform/platform_internal.h"
#include "glad/glad.h"
#include "log.c/log.h"
#include "opengl_platform.h"
#include <GL/gl.h>
#include <GL/glx.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);

GLXContext gl_context;

bool cstrl_opengl_platform_init(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    int attribute_list[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    XVisualInfo *visual_info = glXChooseVisual(state->display, 0, attribute_list);
    if (visual_info == NULL)
    {
        log_fatal("Unable to choose visual");
        return false;
    }
    int visual_attribs[] = {GLX_X_RENDERABLE, True, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, GLX_RENDER_TYPE, GLX_RGBA_BIT,
                            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
                            GLX_ALPHA_SIZE, 8, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, True,
                            // GLX_SAMPLE_BUFFERS  , 1,
                            // GLX_SAMPLES         , 4,
                            None};
    int fb_count;
    GLXFBConfig *config = glXChooseFBConfig(state->display, DefaultScreen(state->display), visual_attribs, &fb_count);
    int attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB,    4,   GLX_CONTEXT_MINOR_VERSION_ARB, 6, GLX_CONTEXT_PROFILE_MASK_ARB,
        GLX_CONTEXT_CORE_PROFILE_BIT_ARB, None};
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
    gl_context = glXCreateContextAttribsARB(state->display, *config, 0, True, attribs);
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
    internal_state *state = platform_state->internal_state;
    glXDestroyContext(state->display, gl_context);
}

void cstrl_opengl_platform_swap_buffers(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;
    glXSwapBuffers(state->display, state->main_window);
}

#endif
