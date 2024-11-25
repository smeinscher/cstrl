//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_PLATFORM_H
#define CSTRL_PLATFORM_H

#include "cstrl_defines.h"
#include <stdbool.h>

typedef struct cstrl_platform_state
{
    void *internal_state;
} cstrl_platform_state;

bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y, int width,
                         int height);

void cstrl_platform_destroy(cstrl_platform_state *platform_state);

bool cstrl_platform_pump_messages(cstrl_platform_state *platform_state);

double cstrl_platform_get_absolute_time();

void cstrl_platform_sleep(unsigned long long ms);

#endif // CSTRL_PLATFORM_H
