//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_RENDERER_H
#define CSTRL_RENDERER_H
#include "cstrl_platform.h"

void cstrl_renderer_init(cstrl_platform_state *platform_state);

void cstrl_renderer_clear(float r, float g, float b, float a);

void cstrl_renderer_draw();

void cstrl_renderer_destroy(cstrl_platform_state *platform_state);

#endif // CSTRL_RENDERER_H
