//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_OPENGL_PLATFORM_H
#define CSTRL_OPENGL_PLATFORM_H
#include "cstrl/cstrl_platform.h"

#include <stdbool.h>

CSTRL_API bool cstrl_opengl_platform_init(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_opengl_platform_destroy(cstrl_platform_state *platform_state);

CSTRL_API void cstrl_opengl_platform_swap_buffers(cstrl_platform_state *platform_state);

#endif // CSTRL_OPENGL_PLATFORM_H
