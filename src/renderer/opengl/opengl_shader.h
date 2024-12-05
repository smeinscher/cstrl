//
// Created by sterling on 5/31/24.
//

#ifndef OPENGL_SHADER_H
#define OPENGL_SHADER_H

#include <cstrl/cstrl_math.h>
#include <sys/time.h>

typedef struct Shader
{
    unsigned int program;
    const char *vertex_shader_path;
    const char *fragment_shader_path;

    time_t vertex_shader_last_modified_timestamp;
    time_t fragment_shader_last_modified_timestamp;
} Shader;

Shader cstrl_opengl_load_shaders_from_files(const char *vertex_shader_path, const char *fragment_shader_path);

Shader cstrl_opengl_load_shaders_from_source(const char *vertex_shader_source, const char *fragment_shader_source);

void cstrl_opengl_use_shader(Shader shader);

void cstrl_opengl_set_uniform_float(unsigned int program, const char *name, float f);

void cstrl_opengl_set_uniform_3f(unsigned int program, const char *name, float x, float y, float z);

void cstrl_opengl_set_uniform_4f(unsigned int program, const char *name, float x, float y, float z, float w);

void cstrl_opengl_set_uniform_mat4(unsigned int program, const char *name, mat4 mat);

void cstrl_opengl_shader_hot_reload(Shader *shader);

#endif // OPENGL_SHADER_H
