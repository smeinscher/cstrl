//
// Created by sterling on 5/31/24.
//

#include "cstrl/cstrl_renderer.h"

#if defined(CSTRL_RENDER_API_OPENGL)

#include <stdio.h>

#include "cstrl/cstrl_util.h"
#if defined(CSTRL_PLATFORM_ANDROID)
#include <glad/gles3/glad.h>
#else
#include "glad/glad.h"
#endif
#include "log.c/log.h"

#include <stdlib.h>

#define MAX_LOG_BUFFER_SIZE 512

unsigned int compile_shader(const char *shader_source, unsigned int type);

CSTRL_API cstrl_shader cstrl_load_shaders_from_files(const char *vertex_shader_path, const char *fragment_shader_path)
{
    long file_size;
    char *vertex_shader_source = cstrl_read_file(vertex_shader_path, &file_size);
    char *fragment_shader_source = cstrl_read_file(fragment_shader_path, &file_size);
    cstrl_shader shader = cstrl_load_shaders_from_source(vertex_shader_source, fragment_shader_source);
    shader.vertex_shader_path = vertex_shader_path;
    shader.fragment_shader_path = fragment_shader_path;
    shader.vertex_shader_last_modified_timestamp = cstrl_get_file_timestamp(vertex_shader_path);
    shader.fragment_shader_last_modified_timestamp = cstrl_get_file_timestamp(fragment_shader_path);

    free(vertex_shader_source);
    free(fragment_shader_source);

    return shader;
}

CSTRL_API cstrl_shader cstrl_load_shaders_from_source(const char *vertex_shader_source,
                                                      const char *fragment_shader_source)
{
    cstrl_shader shader = {0};
    const unsigned int vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    if (vertex_shader == 0)
    {
        log_error("Vertex shader failed to compile");
        return shader;
    }
    const unsigned int fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
    if (fragment_shader == 0)
    {
        log_error("Fragment shader failed to compile");
        glDeleteShader(vertex_shader);
        return shader;
    }

    unsigned int shader_program = glCreateProgram();

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    int success;
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[MAX_LOG_BUFFER_SIZE];
        glGetProgramInfoLog(shader_program, MAX_LOG_BUFFER_SIZE, NULL, info_log);
        log_error("Failed to link shader_program:\n\t%s", info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glUseProgram(shader_program);

    shader.program = shader_program;
    return shader;
}

CSTRL_API cstrl_shader cstrl_load_shaders_tessellation_from_files(const char *vertex_shader_path,
                                                                  const char *fragment_shader_path,
                                                                  const char *tessellation_control_shader_path,
                                                                  const char *tessellation_evaluation_shader_path)
{
    long file_size;
    char *vertex_shader_source = cstrl_read_file(vertex_shader_path, &file_size);
    char *fragment_shader_source = cstrl_read_file(fragment_shader_path, &file_size);
    char *tessellation_control_shader_source = cstrl_read_file(tessellation_control_shader_path, &file_size);
    char *tessellation_evaluation_shader_source = cstrl_read_file(tessellation_evaluation_shader_path, &file_size);
    cstrl_shader shader = cstrl_load_shaders_tessellation_from_source(vertex_shader_source, fragment_shader_source,
                                                                      tessellation_control_shader_source,
                                                                      tessellation_evaluation_shader_source);
    shader.vertex_shader_path = vertex_shader_path;
    shader.fragment_shader_path = fragment_shader_path;
    shader.vertex_shader_last_modified_timestamp = cstrl_get_file_timestamp(vertex_shader_path);
    shader.fragment_shader_last_modified_timestamp = cstrl_get_file_timestamp(fragment_shader_path);

    free(vertex_shader_source);
    free(fragment_shader_source);
    free(tessellation_control_shader_source);
    free(tessellation_evaluation_shader_source);

    return shader;
}

CSTRL_API cstrl_shader cstrl_load_shaders_tessellation_from_source(const char *vertex_shader_source,
                                                                   const char *fragment_shader_source,
                                                                   const char *tessellation_control_shader_source,
                                                                   const char *tessellation_evaluation_shader_source)
{
    cstrl_shader shader = {0};
    const unsigned int vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    if (vertex_shader == 0)
    {
        log_error("Vertex shader failed to compile");
        return shader;
    }
    const unsigned int fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
    if (fragment_shader == 0)
    {
        log_error("Fragment shader failed to compile");
        glDeleteShader(vertex_shader);
        return shader;
    }
    const unsigned int tessellation_control_shader =
        compile_shader(tessellation_control_shader_source, GL_TESS_CONTROL_SHADER);
    if (tessellation_control_shader == 0)
    {
        log_error("Tessellation Control shader failed to compile");
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return shader;
    }
    const unsigned int tessellation_evaluation_shader =
        compile_shader(tessellation_evaluation_shader_source, GL_TESS_EVALUATION_SHADER);
    if (tessellation_evaluation_shader == 0)
    {
        log_error("Tessellation Evaluation shader failed to compile");
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteShader(tessellation_control_shader);
        return shader;
    }

    unsigned int shader_program = glCreateProgram();

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glAttachShader(shader_program, tessellation_control_shader);
    glAttachShader(shader_program, tessellation_evaluation_shader);

    int success;
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[MAX_LOG_BUFFER_SIZE];
        glGetProgramInfoLog(shader_program, MAX_LOG_BUFFER_SIZE, NULL, info_log);
        log_error("Failed to link shader_program:\n\t%s", info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteShader(tessellation_control_shader);
    glDeleteShader(tessellation_evaluation_shader);

    glUseProgram(shader_program);

    shader.program = shader_program;
    return shader;
}

unsigned int compile_shader(const char *shader_source, unsigned int type)
{
    const unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[MAX_LOG_BUFFER_SIZE];
        glGetShaderInfoLog(shader, MAX_LOG_BUFFER_SIZE, NULL, info_log);
        log_error("Failed to compile shader:\n\t%s", info_log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

CSTRL_API void cstrl_use_shader(cstrl_shader shader)
{
    glUseProgram(shader.program);
}

CSTRL_API void cstrl_set_uniform_block_binding(cstrl_shader shader, const char *name, unsigned int binding_point)
{
    unsigned int uniform_block_index = glGetUniformBlockIndex(shader.program, name);
    glUniformBlockBinding(shader.program, uniform_block_index, binding_point);
}

CSTRL_API void cstrl_set_uniform_int(unsigned int program, const char *name, int d)
{
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, name), d);
}

CSTRL_API void cstrl_set_uniform_int_array(unsigned int program, const char *name, int count, int *d)
{
    glUseProgram(program);
    glUniform1iv(glGetUniformLocation(program, name), count, d);
}

CSTRL_API void cstrl_set_uniform_float(unsigned int program, const char *name, float f)
{
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, name), f);
}

CSTRL_API void cstrl_set_uniform_3f(unsigned int program, const char *name, float x, float y, float z)
{
    glUseProgram(program);
    glUniform3f(glGetUniformLocation(program, name), x, y, z);
}

CSTRL_API void cstrl_set_uniform_4f(unsigned int program, const char *name, float x, float y, float z, float w)
{
    glUseProgram(program);
    glUniform4f(glGetUniformLocation(program, name), x, y, z, w);
}

CSTRL_API void cstrl_set_uniform_mat4(unsigned int program, const char *name, mat4 mat)
{
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &mat.m[0]);
}

CSTRL_API void cstrl_shader_hot_reload(cstrl_shader *shader)
{
    time_t vertex_current_timestamp = cstrl_get_file_timestamp(shader->vertex_shader_path);
    time_t fragment_current_timestamp = cstrl_get_file_timestamp(shader->fragment_shader_path);

    if (vertex_current_timestamp > shader->vertex_shader_last_modified_timestamp ||
        fragment_current_timestamp > shader->fragment_shader_last_modified_timestamp)
    {
        log_trace("Hot reloading shader");
        glDeleteProgram(shader->program);
        *shader = cstrl_load_shaders_from_source(shader->vertex_shader_path, shader->fragment_shader_path);
    }
}

#endif
