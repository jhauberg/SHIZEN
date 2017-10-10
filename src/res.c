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

#include <stdlib.h> // NULL
#include <string.h> // strcmp, strrchr

#include "res.h"
#include "io.h"

#ifdef SHIZ_DEBUG
 #include "debug/debug.h"
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

u8 const SHIZResourceInvalid = 0;

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

static
bool
z_res__image_loaded_callback(int width, int height, int components,
                             unsigned char * data);

static
bool
z_res__is_image(u8 resource_id);

static
bool
z_res__is_sound(u8 resource_id);

static
SHIZResourceType
z_res__type_from_id(u8 resource_id);

static
s16 const
z_res__index_from_id(u8 resource_id,
                     SHIZResourceType type);

static
u8 z_res__next_id(SHIZResourceType,
                  u8 * index);

static
bool z_res__is_index_free(u8 resource_index,
                          SHIZResourceType type);

static
char const *
z_res__filename_ext(char const * filename);

static SHIZResourceImage * _current_image_resource; // temporary pointer to the image being loaded

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
z_res__image(u8 const resource_id)
{
    if (resource_id > SHIZResourceImageIdMax) {
#ifdef SHIZ_DEBUG
        if (resource_id == _font_resource.resource_id) {
            return _font_resource;
        }
#endif
        return SHIZResourceImageEmpty;
    }
    
    s16 const index = z_res__index_from_id(resource_id, SHIZResourceTypeImage);
    
    if (index < 0) {
        return SHIZResourceImageEmpty;
    }
    
    return _images[index];
}

SHIZResourceSound
z_res__sound(u8 const resource_id)
{
    if (resource_id > SHIZResourceSoundIdMax) {
        return SHIZResourceSoundEmpty;
    }
    
    s16 const index = z_res__index_from_id(resource_id, SHIZResourceTypeSound);
    
    if (index < 0) {
        return SHIZResourceSoundEmpty;
    }
    
    return _sounds[index];
}

u8
z_res__load(char const * const filename)
{
    SHIZResourceType const type = z_res__type(filename);
    
    if (type == SHIZResourceTypeNotSupported) {
        z_io__error("resource not loaded ('%s'); unsupported type (%s)",
                    filename, z_res__filename_ext(filename));
        
        return SHIZResourceInvalid;
    }
    
    u8 resource_id = SHIZResourceInvalid;
    
    u8 expected_index;
    u8 const expected_id = z_res__next_id(type, &expected_index);
    
    if (expected_id != SHIZResourceInvalid) {
        if (type == SHIZResourceTypeImage) {
            _current_image_resource = &_images[expected_index];
            
            if (z_io__load_image(filename, z_res__image_loaded_callback)) {
                _current_image_resource->resource_id = expected_id;
                _current_image_resource->filename = filename;
                
                resource_id = expected_id;
            }
            
            _current_image_resource = NULL;
        } else if (type == SHIZResourceTypeSound) {
            // todo: actual loading left blank for future implementation
            
            _sounds[expected_index].resource_id = expected_id;
            _sounds[expected_index].filename = filename;
            
            resource_id = expected_id;
        }
    }
    
    return resource_id;
}

u8
z_res__load_data(SHIZResourceType const type,
                 unsigned char const * const buffer,
                 unsigned int const length)
{
    if (type == SHIZResourceTypeNotSupported) {
        z_io__error("resource not loaded; unsupported type");
        
        return SHIZResourceInvalid;
    }
    
    u8 resource_id = SHIZResourceInvalid;

    u8 expected_index;
    u8 const expected_id = z_res__next_id(type, &expected_index);
    
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
z_res__unload(u8 const resource_id)
{
    if (resource_id == SHIZResourceInvalid) {
        return false;
    }
    
    SHIZResourceType const type = z_res__type_from_id(resource_id);
    
    s16 const index = z_res__index_from_id(resource_id, type);
    
    if (index < 0) {
        z_io__error("could not unload resource (%d); index out of bounds (%d)",
                    resource_id, index);
    } else {
        if (type == SHIZResourceTypeImage) {
            glDeleteTextures(1, &_images[index].texture_id);
            
            _images[index].width = 0;
            _images[index].height = 0;
            _images[index].resource_id = SHIZResourceInvalid;
            _images[index].filename = NULL;
            _images[index].texture_id = 0;
            
            return true;
        } else if (type == SHIZResourceTypeSound) {
            // todo: actual unloading left blank for future implementation
            
            _sounds[index].resource_id = SHIZResourceInvalid;
            
            return true;
        } else {
            z_io__error("could not unload resource (%d); resource not found",
                        resource_id);
        }
    }
    
    return false;
}

bool
z_res__unload_all()
{
    bool something_failed = false;
    
    for (u8 image_resource_index = 0; image_resource_index < SHIZResourceImageMax; image_resource_index++) {
        u8 const resource_id = _images[image_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!z_res__unload(resource_id)) {
                something_failed = true;
            }
        }
    }
    
    for (u8 sound_resource_index = 0; sound_resource_index < SHIZResourceSoundMax; sound_resource_index++) {
        u8 const resource_id = _sounds[sound_resource_index].resource_id;
        
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
s16 const
z_res__index_from_id(u8 const resource_id,
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
z_res__type_from_id(u8 const resource_id)
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
z_res__is_image(u8 const resource_id)
{
    return resource_id < SHIZResourceImageIdMax;
}

static
bool
z_res__is_sound(u8 const resource_id)
{
    return !z_res__is_image(resource_id) &&
        resource_id < SHIZResourceSoundIdMax;
}

static
bool
z_res__is_index_free(u8 const resource_index,
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
u8
z_res__next_id(SHIZResourceType const type,
               u8 * const index)
{
    if (type == SHIZResourceTypeNotSupported) {
        return SHIZResourceInvalid;
    }
    
    u8 start_id = 0;
    u8 max_id = 0;
    
    if (type == SHIZResourceTypeImage) {
        start_id = SHIZResourceImageIdOffset;
        max_id = SHIZResourceImageIdMax;
    } else if (type == SHIZResourceTypeSound) {
        start_id = SHIZResourceSoundIdOffset;
        max_id = SHIZResourceSoundIdMax;
    }
    
    u8 next_index = 0;
    
    for (u8 next_id = start_id; next_id < max_id; next_id++) {
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
z_res__image_loaded_callback(int const width,
                             int const height,
                             int const components,
                             unsigned char * const data)
{
    if (_current_image_resource == NULL) {
        return false;
    }
    
    if ((width <= 0 || height <= 0) ||
        (width > UINT16_MAX || height > UINT16_MAX)) {
        return false;
    }
    
    _current_image_resource->width = (u16)width;
    _current_image_resource->height = (u16)height;
    
    glGenTextures(1, &_current_image_resource->texture_id);
    glBindTexture(GL_TEXTURE_2D, _current_image_resource->texture_id); {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        if (components == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (components == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
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
    for (u8 image_resource_index = 0; image_resource_index < SHIZResourceImageMax; image_resource_index++) {
        u8 resource_id = _images[image_resource_index].resource_id;
        
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
    for (u8 sound_resource_index = 0; sound_resource_index < SHIZResourceSoundMax; sound_resource_index++) {
        u8 resource_id = _sounds[sound_resource_index].resource_id;
        
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
z_debug__load_font(unsigned char const * const buffer,
                   unsigned int const length)
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

u8
z_debug__get_font_resource()
{
    return _font_resource.resource_id;
}

#endif
