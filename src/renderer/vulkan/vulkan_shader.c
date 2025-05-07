//
// Created by 12105 on 12/13/2024.
//

#if defined(CSTRL_RENDER_API_VULKAN)

#include "cstrl/cstrl_renderer.h"

cstrl_shader cstrl_load_shaders_from_files(const char *vertex_shader_path, const char *fragment_shader_path)
{
    return cstrl_load_shaders_from_source(NULL, NULL);
}

cstrl_shader cstrl_load_shaders_from_source(const char *vertex_shader_source, const char *fragment_shader_source)
{
    cstrl_shader shader = {0};
    return shader;
}

void cstrl_use_shader(cstrl_shader shader)
{
}

void cstrl_set_uniform_float(unsigned int program, const char *name, float f)
{
}

void cstrl_set_uniform_3f(unsigned int program, const char *name, float x, float y, float z)
{
}

void cstrl_set_uniform_4f(unsigned int program, const char *name, float x, float y, float z, float w)
{
}

void cstrl_set_uniform_mat4(unsigned int program, const char *name, mat4 mat)
{
}

void cstrl_shader_hot_reload(cstrl_shader *shader)
{
}

#endif
