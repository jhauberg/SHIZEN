////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2016 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#include "res.h" // SHIZResource*, z_res_*

#include <stdlib.h> // NULL
#include <stdbool.h> // bool
#include <stdint.h> // uint8_t, uint16_t, int32_t
#include <string.h> // strcmp, strrchr

#include "graphics/gfx.h"
#include "mixer.h"

#include "io.h"

#ifdef SHIZ_DEBUG
 #include "debug/debug.h"
 #include <stdio.h> // printf
#endif

SHIZResourceImage const SHIZResourceImageEmpty = {
    .resource_id = 0,
    .width = 0,
    .height = 0,
    .texture_id = 0,
    .filename = NULL
};

SHIZResourceSound const SHIZResourceSoundEmpty = {
    .resource_id = 0,
    .filename = NULL
};

uint8_t const SHIZResourceInvalid = 0;

#define SHIZResourceImageMax 16
#define SHIZResourceSoundMax 8

// offset by 1 to skip the invalid resource id (0); index 0 still used
#define SHIZResourceImageIdOffset 1
#define SHIZResourceImageIdMax (SHIZResourceImageIdOffset + SHIZResourceImageMax)

#define SHIZResourceSoundIdOffset SHIZResourceImageIdMax
#define SHIZResourceSoundIdMax (SHIZResourceSoundIdOffset + SHIZResourceSoundMax)

#ifdef SHIZ_DEBUG
 #define SHIZResourceIdMax SHIZResourceSoundIdMax
#endif

static bool z_res__image_loaded_callback(int32_t width, int32_t height, int32_t components, uint8_t * data);
static bool z_res__sound_loaded_callback(int32_t channels, int32_t sample_rate, int16_t * data, int32_t size);

static bool z_res__is_image(uint8_t resource_id);
static bool z_res__is_sound(uint8_t resource_id);
static bool z_res__is_index_free(uint8_t resource_index, SHIZResourceType);

static SHIZResourceType z_res__type_from_id(uint8_t resource_id);
static int16_t const z_res__index_from_id(uint8_t resource_id, SHIZResourceType);

static uint8_t z_res__next_id(SHIZResourceType, uint8_t * index);

static char const * z_res__filename_ext(char const * filename);

static SHIZResourceImage * _current_image_resource; // temporary pointer to the image being loaded
static SHIZResourceSound * _current_sound_resource; // temporary pointer to the sound being loaded

static SHIZResourceImage _images[SHIZResourceImageMax];
static SHIZResourceSound _sounds[SHIZResourceSoundMax];

#ifdef SHIZ_DEBUG
static SHIZResourceImage _font_resource;
#endif

SHIZResourceType const
z_res__type(char const * const filename)
{
    SHIZResourceType resource_type = SHIZResourceTypeNotSupported;
    
    char const * const extension = z_res__filename_ext(filename);
    
    if (strcmp("png", extension) == 0) {
        resource_type = SHIZResourceTypeImage;
    } else if (strcmp("ogg", extension) == 0) {
        resource_type = SHIZResourceTypeSound;
    }
    
    return resource_type;
}

SHIZResourceImage
z_res__image(uint8_t const resource_id)
{
    if (resource_id > SHIZResourceImageIdMax) {
#ifdef SHIZ_DEBUG
        if (resource_id == _font_resource.resource_id) {
            return _font_resource;
        }
#endif
        return SHIZResourceImageEmpty;
    }
    
    int16_t const index = z_res__index_from_id(resource_id, SHIZResourceTypeImage);
    
    if (index < 0) {
        return SHIZResourceImageEmpty;
    }
    
    return _images[index];
}

SHIZResourceSound
z_res__sound(uint8_t const resource_id)
{
    if (resource_id > SHIZResourceSoundIdMax) {
        return SHIZResourceSoundEmpty;
    }
    
    int16_t const index = z_res__index_from_id(resource_id, SHIZResourceTypeSound);
    
    if (index < 0) {
        return SHIZResourceSoundEmpty;
    }
    
    return _sounds[index];
}

uint8_t
z_res__load(char const * const filename)
{
    SHIZResourceType const type = z_res__type(filename);
    
    if (type == SHIZResourceTypeNotSupported) {
        z_io__error("resource not loaded ('%s'); unsupported type (%s)",
                    filename, z_res__filename_ext(filename));
        
        return SHIZResourceInvalid;
    }
    
    uint8_t resource_id = SHIZResourceInvalid;
    
    uint8_t expected_index;
    uint8_t const expected_id = z_res__next_id(type, &expected_index);
    
    if (expected_id != SHIZResourceInvalid) {
        if (type == SHIZResourceTypeImage) {
            _current_image_resource = &_images[expected_index];
            _current_image_resource->resource_id = expected_id;
            _current_image_resource->filename = filename;
            
            if (z_io__load_image(filename, z_res__image_loaded_callback)) {
                resource_id = expected_id;
            }
            
            _current_image_resource = NULL;
        } else if (type == SHIZResourceTypeSound) {
            _current_sound_resource = &_sounds[expected_index];
            _current_sound_resource->resource_id = expected_id;
            _current_sound_resource->filename = filename;
            
            if (z_io__load_sound(filename, z_res__sound_loaded_callback)) {
                resource_id = expected_id;
            }
            
            _current_sound_resource = NULL;
        }
    }
    
    return resource_id;
}

uint8_t
z_res__load_data(SHIZResourceType const type,
                 uint8_t const * const buffer,
                 uint32_t const length)
{
    if (type == SHIZResourceTypeNotSupported) {
        z_io__error("resource not loaded; unsupported type");
        
        return SHIZResourceInvalid;
    }
    
    uint8_t resource_id = SHIZResourceInvalid;

    uint8_t expected_index;
    uint8_t const expected_id = z_res__next_id(type, &expected_index);
    
    if (expected_id != SHIZResourceInvalid) {
        if (type == SHIZResourceTypeImage) {
            _current_image_resource = &_images[expected_index];
            
            if (z_io__load_image_data(buffer, length, z_res__image_loaded_callback)) {
                _current_image_resource->resource_id = expected_id;
                _current_image_resource->filename = NULL;
                
                resource_id = expected_id;
            }
            
            _current_image_resource = NULL;
        } else if (type == SHIZResourceTypeSound) {
            // todo: actual loading left blank for future implementation
            
            _sounds[expected_index].resource_id = expected_id;
            
            resource_id = expected_id;
        }
    }
    
    return resource_id;
}

bool
z_res__unload(uint8_t const resource_id)
{
    if (resource_id == SHIZResourceInvalid) {
        return false;
    }
    
    bool unloaded = false;
    
    SHIZResourceType const type = z_res__type_from_id(resource_id);
    
    int16_t const index = z_res__index_from_id(resource_id, type);
    
    if (index < 0) {
        z_io__error("could not unload resource (%d); index out of bounds (%d)",
                    resource_id, index);
    } else {
        if (type == SHIZResourceTypeImage) {
            unloaded = z_gfx__destroy_texture(&_images[index]);
            
            if (!unloaded) {
                z_io__error("could not unload image (%d)", resource_id);
            }
            
            _images[index].width = 0;
            _images[index].height = 0;
            _images[index].resource_id = SHIZResourceInvalid;
            _images[index].filename = NULL;
            _images[index].texture_id = 0;
        } else if (type == SHIZResourceTypeSound) {
            unloaded = z_mixer__destroy_sound(&_sounds[index]);
            
            if (!unloaded) {
                z_io__error("could not unload sound (%d)", resource_id);
            }
            
            _sounds[index].resource_id = SHIZResourceInvalid;
            _sounds[index].filename = NULL;
            _sounds[index].source_id = 0;
        } else {
            z_io__error("could not unload resource (%d); resource not found",
                        resource_id);
        }
    }
    
    return unloaded;
}

bool
z_res__unload_all()
{
    bool something_failed = false;
    
    for (uint8_t image_resource_index = 0; image_resource_index < SHIZResourceImageMax; image_resource_index++) {
        uint8_t const resource_id = _images[image_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!z_res__unload(resource_id)) {
                something_failed = true;
            }
        }
    }
    
    for (uint8_t sound_resource_index = 0; sound_resource_index < SHIZResourceSoundMax; sound_resource_index++) {
        uint8_t const resource_id = _sounds[sound_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!z_res__unload(resource_id)) {
                something_failed = true;
            }
        }
    }
    
#ifdef SHIZ_DEBUG
    if (_font_resource.resource_id != SHIZResourceInvalid) {
        if (!z_debug__unload_font()) {
            something_failed = true;
        }
    }
#endif
    
    return !something_failed;
}

static
int16_t const
z_res__index_from_id(uint8_t const resource_id,
                     SHIZResourceType const type)
{
    if (type == SHIZResourceTypeImage) {
        return resource_id - SHIZResourceImageIdOffset;
    } else if (type == SHIZResourceTypeSound) {
        return resource_id - SHIZResourceSoundIdOffset;
    }
    
    return -1;
}

static
SHIZResourceType
z_res__type_from_id(uint8_t const resource_id)
{
    if (z_res__is_image(resource_id)) {
        return SHIZResourceTypeImage;
    } else if (z_res__is_sound(resource_id)) {
        return SHIZResourceTypeSound;
    }
    
    return SHIZResourceTypeNotSupported;
}

static
bool
z_res__is_image(uint8_t const resource_id)
{
    return resource_id < SHIZResourceImageIdMax;
}

static
bool
z_res__is_sound(uint8_t const resource_id)
{
    return !z_res__is_image(resource_id) &&
        resource_id < SHIZResourceSoundIdMax;
}

static
bool
z_res__is_index_free(uint8_t const resource_index,
                     SHIZResourceType const type)
{
    if (type == SHIZResourceTypeImage) {
        return _images[resource_index].resource_id == SHIZResourceInvalid;
    } else if (type == SHIZResourceTypeSound) {
        return _sounds[resource_index].resource_id == SHIZResourceInvalid;
    }
    
    return false;
}

static
uint8_t
z_res__next_id(SHIZResourceType const type,
               uint8_t * const index)
{
    if (type == SHIZResourceTypeNotSupported) {
        return SHIZResourceInvalid;
    }
    
    uint8_t start_id = 0;
    uint8_t max_id = 0;
    
    if (type == SHIZResourceTypeImage) {
        start_id = SHIZResourceImageIdOffset;
        max_id = SHIZResourceImageIdMax;
    } else if (type == SHIZResourceTypeSound) {
        start_id = SHIZResourceSoundIdOffset;
        max_id = SHIZResourceSoundIdMax;
    }
    
    uint8_t next_index = 0;
    
    for (uint8_t next_id = start_id; next_id < max_id; next_id++) {
        next_index = next_id - start_id;
        
        if (z_res__is_index_free(next_index, type)) {
            *index = next_index;
            
            return next_id;
        }
    }
    
    if (type == SHIZResourceTypeImage) {
        z_io__error("image limit reached (%d)", SHIZResourceImageMax);
    } else if (type == SHIZResourceTypeSound) {
        z_io__error("sound limit reached (%d)", SHIZResourceSoundMax);
    }
    
    return SHIZResourceInvalid;
}

static
bool
z_res__image_loaded_callback(int32_t const width,
                             int32_t const height,
                             int32_t const components,
                             uint8_t * const data)
{
    if (_current_image_resource == NULL) {
        return false;
    }
    
    if (data == NULL ||
        (width <= 0 || height <= 0) ||
        (width > UINT16_MAX || height > UINT16_MAX)) {
        return false;
    }
    
    _current_image_resource->width = (uint16_t)width;
    _current_image_resource->height = (uint16_t)height;

    return z_gfx__create_texture(_current_image_resource,
                                 width, height,
                                 components,
                                 data);
}

static
bool
z_res__sound_loaded_callback(int32_t const channels,
                             int32_t const sample_rate,
                             int16_t * const data,
                             int32_t size)
{
    if (_current_sound_resource == NULL) {
        return false;
    }
    
    if (data == NULL) {
        return false;
    }
    
    return z_mixer__create_sound(_current_sound_resource,
                                 channels, sample_rate,
                                 data, size);
}

static
char const *
z_res__filename_ext(char const * const filename)
{
    char const * const extension_index = strrchr(filename, '.');
    
    if (!extension_index || extension_index == filename) {
        return NULL;
    }
    
    return extension_index + 1; // extension without the '.'
}

#ifdef SHIZ_DEBUG

void
z_debug__print_resources()
{
    printf("  IDX  ID  RESOURCE\n");
    printf("  -----------------\n");
    for (uint8_t image_resource_index = 0; image_resource_index < SHIZResourceImageMax; image_resource_index++) {
        uint8_t resource_id = _images[image_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            printf("i %02d: [%02d] %s (%dx%d)\n",
                   image_resource_index, resource_id,
                   _images[image_resource_index].filename,
                   _images[image_resource_index].width,
                   _images[image_resource_index].height);
        } else {
            printf("i %02d: [%02d] ---\n", image_resource_index, resource_id);
        }
    }
    for (uint8_t sound_resource_index = 0; sound_resource_index < SHIZResourceSoundMax; sound_resource_index++) {
        uint8_t resource_id = _sounds[sound_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            printf("s %02d: [%02d] %s\n",
                   sound_resource_index, resource_id,
                   _sounds[sound_resource_index].filename);
        } else {
            printf("s %02d: [%02d] ---\n", sound_resource_index, resource_id);
        }
    }
}

bool
z_debug__load_font(uint8_t const * const buffer,
                   uint32_t const length)
{
    if (_font_resource.resource_id != SHIZResourceInvalid) {
        return false;
    }
    
    _current_image_resource = &_font_resource;
    
    if (z_io__load_image_data(buffer, length, z_res__image_loaded_callback)) {
        // offset by 1 to ensure the id won't be occupied by any other resource
        _font_resource.resource_id = SHIZResourceIdMax + 1;
    }
    
    _current_image_resource = NULL;
    
    return true;
}

bool
z_debug__unload_font()
{
    if (_font_resource.texture_id == 0) {
        return false;
    }
    
    glDeleteTextures(1, &_font_resource.texture_id);
    
    _font_resource.width = 0;
    _font_resource.height = 0;
    _font_resource.resource_id = SHIZResourceInvalid;
    _font_resource.filename = NULL;
    _font_resource.texture_id = 0;
    
    return true;
}

uint8_t
z_debug__get_font_resource()
{
    return _font_resource.resource_id;
}

#endif
