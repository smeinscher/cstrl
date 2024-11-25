//
// Created by sterling on 6/9/24.
//

#ifndef OPENGL_TEXTURE_H
#define OPENGL_TEXTURE_H
#include <time.h>

typedef struct Texture
{
    unsigned int id;
    const char *path;
    time_t last_modified_timestamp;
} Texture;

Texture generate_opengl_texture(const char *path);

void opengl_texture_hot_reload(Texture *texture);

#endif // OPENGL_TEXTURE_H
