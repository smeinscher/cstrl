#ifndef CSTRL_AUDIO_H
#define CSTRL_AUDIO_H

#include "cstrl/cstrl_defines.h"
#include <stdbool.h>

typedef struct sound_t
{
    unsigned int buffer;
} sound_t;

typedef struct source_t
{
    unsigned int id;
} source_t;

CSTRL_API bool cstrl_audio_init();

CSTRL_API bool cstrl_audio_load_ogg(const char *path, sound_t *sound_out);

CSTRL_API void cstrl_audio_unload(sound_t *sound);

CSTRL_API bool cstrl_audio_create_source(source_t *source_out);

CSTRL_API void cstrl_audio_play(source_t *source, sound_t *sound, bool loop);

CSTRL_API void cstrl_audio_stop(source_t *source);

CSTRL_API void cstrl_audio_set_position(source_t *source, float x, float y, float z);

CSTRL_API void cstrl_audio_set_listener_position(float x, float y, float z);

CSTRL_API void cstrl_audio_shutdown();

#endif // CSTRL_AUDIO_H
