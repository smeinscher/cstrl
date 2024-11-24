//
// Created by 12105 on 11/23/2024.
//

#include "cstrl_opengl_platform.h"
#include "glad/glad.h"

#include <cstrl/cstrl_renderer.h>

void cstrl_renderer_init(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_init(platform_state);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_WIDTH);
    glLineWidth(1.0f);
}

void cstrl_renderer_clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void cstrl_renderer_draw()
{
}

void cstrl_renderer_destroy(cstrl_platform_state *platform_state)
{
    cstrl_opengl_platform_destroy(platform_state);
}
