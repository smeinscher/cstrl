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

CSTRL_API cstrl_texture cstrl_texture_generate_from_bitmap(unsigned char *bitmap, int width, int height)
{
    cstrl_texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

    glGenerateMipmap(GL_TEXTURE_2D);

    texture.id = texture_id;
    texture.path = "";
    texture.last_modified_timestamp = 0;
    return texture;
}

CSTRL_API cstrl_texture cstrl_texture_cube_map_generate_from_folder(const char *folder)
{
    cstrl_texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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
