////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2017 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#include "mixer.h" // z_mixer_*
#include "res.h" // SHIZResourceSound
#include "io.h" // z_io_*

#include <stdlib.h> // NULL
#include <stdint.h> // uint8_t, int16_t, in32_t

#include "internal.h" // ALCdevice, ALCcontext, ALenum, al*

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#ifdef SHIZ_DEBUG
static
void
z_mixer__process_errors(void);
#endif

static ALCdevice * _device;
static ALCcontext * _context;

bool
z_mixer__init()
{
    _device = alcOpenDevice(NULL);
    
    if (!_device) {
#ifdef SHIZ_DEBUG
        z_mixer__process_errors();
#endif
        return false;
    }
    
    _context = alcCreateContext(_device, NULL);
    
    if (!alcMakeContextCurrent(_context)) {
#ifdef SHIZ_DEBUG
        z_mixer__process_errors();
#endif
        return false;
    }
    
    ALfloat const orientation[6] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f
    };
    
    alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, orientation);
    
    return true;
}

bool
z_mixer__kill()
{
    if (_device == NULL) {
        return false;
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(_context);
    
    if (!alcCloseDevice(_device)) {
#ifdef SHIZ_DEBUG
        z_mixer__process_errors();
#endif
        return false;
    }
    
    return true;
}

void
z_mixer__play_sound(uint8_t const sound_resource_id)
{
    SHIZResourceSound const resource = z_res__sound(sound_resource_id);
    
    alSourcePlay(resource.source_id);
#ifdef SHIZ_DEBUG
    z_mixer__process_errors();
#endif
}

void
z_mixer__stop_sound(uint8_t const sound_resource_id)
{
    SHIZResourceSound const resource = z_res__sound(sound_resource_id);
    
    alSourceStop(resource.source_id);
#ifdef SHIZ_DEBUG
    z_mixer__process_errors();
#endif
}

bool
z_mixer__create_sound(SHIZResourceSound * const resource,
                      int32_t const channels,
                      int32_t const sample_rate,
                      int16_t * const data,
                      int32_t size)
{
    if (resource == NULL) {
        return false;
    }
    
    alGenSources(1, &resource->source_id);
    alGenBuffers(1, &resource->buffer_id);
    
    alSourcef(resource->source_id, AL_PITCH, 1);
    alSourcef(resource->source_id, AL_GAIN, 1);
    alSource3f(resource->source_id, AL_POSITION, 0, 0, 0);
    alSource3f(resource->source_id, AL_VELOCITY, 0, 0, 0);
    alSourcei(resource->source_id, AL_LOOPING, AL_FALSE);
    
    bool stereo = channels > 1;
    
    ALenum format = stereo ?
        AL_FORMAT_STEREO16 :
        AL_FORMAT_MONO16;
    
    alBufferData(resource->buffer_id,
                 format, data, size, sample_rate);

    alSourcei(resource->source_id, AL_BUFFER,
              (ALint)resource->buffer_id);
#ifdef SHIZ_DEBUG
    z_mixer__process_errors();
#endif
    return true;
}

bool
z_mixer__destroy_sound(SHIZResourceSound const * const resource)
{
    if (resource == NULL) {
        return false;
    }
    
    if (resource->source_id != 0) {
        alDeleteSources(1, &resource->source_id);
    }
    
    if (resource->buffer_id != 0) {
        alDeleteBuffers(1, &resource->buffer_id);
    }
#ifdef SHIZ_DEBUG
    z_mixer__process_errors();
#endif
    return true;
}

#ifdef SHIZ_DEBUG
static
void
z_mixer__process_errors()
{
    ALCenum error;
    
    while ((error = alGetError()) != AL_NO_ERROR) {
        z_io__error_context("OPENAL", "%d", error);
    }
}
#endif
