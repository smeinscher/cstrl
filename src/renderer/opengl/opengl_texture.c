//
// Created by sterling on 6/9/24.
//

#include "opengl_texture.h"

#include "../../util/file_helpers.h"
#include "glad/glad.h"
#include "log.c/log.h"
#include "stb/stb_image.h"

int upload_opengl_texture(const char *path)
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

Texture generate_opengl_texture(const char *path)
{
    Texture texture = {0};
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    if (upload_opengl_texture(path))
    {
        return texture;
    }

    texture.id = texture_id;
    texture.path = path;
    texture.last_modified_timestamp = get_file_timestamp(path);
    return texture;
}

void opengl_texture_hot_reload(Texture *texture)
{
    time_t current_timestamp = get_file_timestamp(texture->path);

    if (current_timestamp > texture->last_modified_timestamp)
    {
        log_trace("Hot reloading texture");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        upload_opengl_texture(texture->path);
    }
}
