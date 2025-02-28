//
// Created by sterling on 6/9/24.
//

#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_renderer.h"
#include <string.h>

#if defined(CSTRL_RENDER_API_OPENGL)

#include "cstrl/cstrl_util.h"
#if defined(CSTRL_PLATFORM_ANDROID)
#include <glad/gles3/glad.h>
#else
#include "glad/glad.h"
#endif
#include "log.c/log.h"
#include "stb/stb_image.h"

CSTRL_API int upload_opengl_texture(const char *path)
{
    stbi_set_flip_vertically_on_load(true);
    int nr_channels;
    int width, height;
    unsigned char *data = stbi_load(path, &width, &height, &nr_channels, STBI_rgb_alpha);
    if (data == NULL)
    {
        log_error("Failed to load texture: %s", path);
        return -1;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return 0;
}

CSTRL_API cstrl_texture cstrl_texture_framebuffer_generate(int width, int height)
{
    cstrl_texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);
    texture.id = texture_id;
    return texture;
}

CSTRL_API cstrl_texture cstrl_texture_generate_from_path(const char *path)
{
    cstrl_texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (upload_opengl_texture(path))
    {
        return texture;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    texture.id = texture_id;
    texture.path = path;
    texture.last_modified_timestamp = cstrl_get_file_timestamp(path);
    return texture;
}

CSTRL_API cstrl_texture cstrl_texture_generate_from_bitmap(unsigned char *bitmap, int width, int height,
                                                           cstrl_texture_format format,
                                                           cstrl_texture_format internal_format)
{
    cstrl_texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint gl_format;
    GLuint gl_internal_format;
    switch (format)
    {
    case CSTRL_RED:
        gl_format = GL_RED;
        break;
    case CSTRL_RGB:
        gl_format = GL_RGB;
        break;
    case CSTRL_RGBA:
        gl_format = GL_RGBA;
        break;
    }
    switch (internal_format)
    {
    case CSTRL_RED:
        gl_internal_format = GL_R8;
        break;
    case CSTRL_RGB:
        gl_internal_format = GL_RGB;
        break;
    case CSTRL_RGBA:
        gl_internal_format = GL_RGBA;
        break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, bitmap);

    glGenerateMipmap(GL_TEXTURE_2D);

    texture.id = texture_id;
    texture.path = "";
    texture.last_modified_timestamp = 0;
    return texture;
}

CSTRL_API cstrl_texture cstrl_texture_cube_map_generate_from_folder(const char *folder, bool alpha_channel)
{
    cstrl_texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const char *file_names[] = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};
    size_t folder_name_length = strlen(folder);
    for (int i = 0; i < 6; i++)
    {
        char *path = malloc(folder_name_length + strlen(file_names[i]) + 1);
        strcpy(path, folder);
        strcat(path, file_names[i]);
        int width, height, nr_channels;
        unsigned char *data = stbi_load(path, &width, &height, &nr_channels, STBI_rgb_alpha);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, alpha_channel ? GL_RGBA : GL_RGB, width, height, 0,
                     alpha_channel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        free(path);
        stbi_image_free(data);
    }

    texture.id = texture_id;
    texture.path = "";
    texture.last_modified_timestamp = 0;
    return texture;
}

CSTRL_API void cstrl_texture_hot_reload(cstrl_texture *texture)
{
    time_t current_timestamp = cstrl_get_file_timestamp(texture->path);

    if (current_timestamp > texture->last_modified_timestamp)
    {
        log_trace("Hot reloading texture");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        upload_opengl_texture(texture->path);
    }
}

CSTRL_API void cstrl_texture_bind(cstrl_texture texture)
{
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

CSTRL_API void cstrl_texture_cube_map_bind(cstrl_texture texture)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.id);
}

CSTRL_API void cstrl_set_active_texture(unsigned int active_texture)
{
    glActiveTexture(GL_TEXTURE0 + active_texture);
}

#endif
