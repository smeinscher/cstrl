#include "cstrl/cstrl_audio.h"
#include "log.c/log.h"
#include <al.h>
#include <alc.h>
#define STB_VORBIS_IMPLEMENTATION
#include <stb/stb_vorbis.c>

static ALCdevice *g_device = NULL;
static ALCcontext *g_context = NULL;

CSTRL_API bool cstrl_audio_init()
{
    g_device = alcOpenDevice(NULL);
    if (!g_device)
    {
        log_error("Failed to open audio device");
        return false;
    }

    g_context = alcCreateContext(g_device, NULL);
    if (!g_context)
    {
        log_error("Failed to create audio context");
        alcCloseDevice(g_device);
        return false;
    }

    if (!alcMakeContextCurrent(g_context))
    {
        log_error("Failed to make default audio context current");
        alcDestroyContext(g_context);
        alcCloseDevice(g_device);
        return false;
    }

    return true;
}

CSTRL_API bool cstrl_audio_load_ogg(const char *path, sound_t *sound_out)
{
    int channels, sample_rate;
    short *output;
    int samples = stb_vorbis_decode_filename(path, &channels, &sample_rate, &output);
    if (samples < 0)
    {
        log_error("Failed to decode file");
        return false;
    }

    ALenum format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    alGenBuffers(1, &sound_out->buffer);
    alBufferData(sound_out->buffer, format, output, samples * channels * sizeof(short), sample_rate);

    free(output);
    return true;
}

CSTRL_API void cstrl_audio_unload(sound_t *sound)
{
    alDeleteBuffers(1, &sound->buffer);
}

CSTRL_API bool cstrl_audio_create_source(source_t *source_out)
{
    alGenSources(1, &source_out->id);
    return alGetError() == AL_NO_ERROR;
}

CSTRL_API void cstrl_audio_play(source_t *source, sound_t *sound, bool loop)
{
    alSourcei(source->id, AL_BUFFER, sound->buffer);
    alSourcei(source->id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcePlay(source->id);
}

CSTRL_API void cstrl_audio_stop(source_t *source)
{
    alSourceStop(source->id);
}

CSTRL_API void cstrl_audio_set_position(source_t *source, float x, float y, float z)
{
    alSource3f(source->id, AL_POSITION, x, y, z);
}

CSTRL_API void cstrl_audio_set_listener_position(float x, float y, float z)
{
    alListener3f(AL_POSITION, x, y, z);
}

CSTRL_API void cstrl_audio_shutdown()
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(g_context);
    alcCloseDevice(g_device);
}
