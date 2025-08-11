//
// Created by 12105 on 11/23/2024.
//

#include "log.c/log.h"
#include <cstrl/cstrl_defines.h>

#if defined(CSTRL_RENDER_API_OPENGL) && defined(CSTRL_PLATFORM_WINDOWS)
#include "glad/glad.h"
#include "opengl_platform.h"

#include "../../platform/platform_internal.h"
#include "cstrl/cstrl_platform.h"

/*#include <GL/wglext.h>*/
#include <gl/gl.h>
#include <windows.h>

CSTRL_API DWORD NvOptimusEnablement = 0x00000001;

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC h_share_context, const int *attrib_list);
typedef bool WINAPI wglSwapIntervalEXT_type(int interval);

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

HDC dc;
HGLRC rc;

CSTRL_API bool cstrl_opengl_platform_init(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), //  size of this pfd
        1,                             // version number
        PFD_DRAW_TO_WINDOW |           // support window
            PFD_SUPPORT_OPENGL |       // support OpenGL
            PFD_DOUBLEBUFFER,          // double buffered
        PFD_TYPE_RGBA,                 // RGBA type
        24,                            // 24-bit color depth
        0,
        0,
        0,
        0,
        0,
        0, // color bits ignored
        0, // no alpha buffer
        0, // shift bit ignored
        0, // no accumulation buffer
        0,
        0,
        0,
        0,              // accum bits ignored
        32,             // 32-bit z-buffer
        0,              // no stencil buffer
        0,              // no auxiliary buffer
        PFD_MAIN_PLANE, // main layer
        0,              // reserved
        0,
        0,
        0 // layer masks ignored
    };
    dc = GetDC(state->hwnd);
    int pf = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, pf, &pfd);
    HGLRC temp_context = wglCreateContext(dc);
    wglMakeCurrent(dc, temp_context);
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,    4, WGL_CONTEXT_MINOR_VERSION_ARB, 6, WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0};
    wglCreateContextAttribsARB_type *wglCreateContextAttribsARB =
        (wglCreateContextAttribsARB_type *)wglGetProcAddress("wglCreateContextAttribsARB");
    rc = wglCreateContextAttribsARB(dc, 0, attribs);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temp_context);
    wglMakeCurrent(dc, rc);

    gladLoadGL();
    glViewport(0, 0, state->state_common.window_width, state->state_common.window_height);

    wglSwapIntervalEXT_type *wglSwapIntervalEXT = (wglSwapIntervalEXT_type *)wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(1);

    return true;
}

CSTRL_API void cstrl_opengl_platform_destroy(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    ReleaseDC(state->hwnd, dc);
    wglDeleteContext(rc);
}

CSTRL_API void cstrl_opengl_platform_swap_buffers(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    HDC dc = GetDC(state->hwnd);
    SwapBuffers(dc);
}

#endif
