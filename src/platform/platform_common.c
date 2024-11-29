//
// Created by 12105 on 11/27/2024.
//

#include "cstrl/cstrl_platform.h"
#include "platform_internal.h"

void cstrl_platform_set_key_callback(cstrl_platform_state *platform_state, cstrl_key_callback key_callback)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.callbacks.key = key_callback;
}

void cstrl_platform_set_mouse_position_callback(cstrl_platform_state *platform_state,
                                                cstrl_mouse_position_callback mouse_position_callback)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.callbacks.mouse_position = mouse_position_callback;
}
