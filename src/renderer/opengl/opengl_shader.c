//
// Created by sterling on 5/31/24.
//

#include "cstrl/cstrl_renderer.h"

#ifdef CSTRL_RENDERER_OPENGL

#include <stdio.h>

#include "cstrl/cstrl_util.h"
#ifdef CSTRL_PLATFORM_ANDROID
#include <glad/gles3/glad.h>
#else
#include "glad/glad.h"
#endif
#include "log.c/log.h"

#include <stdlib.h>

#define MAX_LOG_BUFFER_SIZE 512

unsigned int compile_shader(const char *shader_source, unsigned int type);

cstrl_shader cstrl_load_shaders_from_files(const char *vertex_shader_path, const char *fragment_shader_path)
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

cstrl_shader cstrl_load_shaders_from_source(const char *vertex_shader_source, const char *fragment_shader_source)
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

char *get_file_contents(const char *path)
{
    char *source = NULL;
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        log_error("Failed to open file %s", path);
        return 0;
    }

    if (fseek(fp, 0L, SEEK_END) == 0)
    {
        const long buffer_size = ftell(fp);
        if (buffer_size == -1)
        {
            log_error("Failed to get size of file");
            return 0;
        }
        source = malloc(sizeof(char) * (buffer_size + 1));
        if (fseek(fp, 0L, SEEK_SET) != 0)
        {
            log_error("Failed to return to start of file\n");
            return 0;
        }

        size_t new_length = fread(source, sizeof(char), buffer_size, fp);
        if (ferror(fp) != 0)
        {
            log_error("Failed to read file");
            return 0;
        }
        source[new_length++] = '\0';
    }
    fclose(fp);

    return source;
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
        return 0;
    }
    return shader;
}

void cstrl_use_shader(cstrl_shader shader)
{
    glUseProgram(shader.program);
}

void cstrl_set_uniform_float(unsigned int program, const char *name, float f)
{
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, name), f);
}

void cstrl_set_uniform_3f(unsigned int program, const char *name, float x, float y, float z)
{
    glUseProgram(program);
    glUniform3f(glGetUniformLocation(program, name), x, y, z);
}

void cstrl_set_uniform_4f(unsigned int program, const char *name, float x, float y, float z, float w)
{
    glUseProgram(program);
    glUniform4f(glGetUniformLocation(program, name), x, y, z, w);
}

void cstrl_set_uniform_mat4(unsigned int program, const char *name, mat4 mat)
{
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &mat.m[0]);
}

void cstrl_shader_hot_reload(cstrl_shader *shader)
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