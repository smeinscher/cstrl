//
// Created by 12105 on 12/13/2024.
//

#if defined(CSTRL_RENDER_API_VULKAN)

#include "cstrl/cstrl_renderer.h"

cstrl_texture cstrl_texture_generate_from_path(const char *path)
{
    cstrl_texture texture = {0};
    return texture;
}

cstrl_texture cstrl_texture_generate_from_bitmap(unsigned char *bitmap, int width, int height)
{
    cstrl_texture texture = {0};
    return texture;
}

void cstrl_texture_hot_reload(cstrl_texture *texture)
{
}

void cstrl_texture_bind(cstrl_texture texture)
{
}

#endif