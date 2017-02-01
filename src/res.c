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

#include <string.h>

#include "res.h"
#include "io.h"

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

unsigned int const SHIZResourceInvalid = 0;

#define SHIZResourceImageMax 16
#define SHIZResourceSoundMax 8

#define SHIZResourceImageIdOffset 1 // offset by 1 to skip the invalid resource id (0); index 0 still used
#define SHIZResourceImageIdMax (SHIZResourceImageIdOffset + SHIZResourceImageMax)

#define SHIZResourceSoundIdOffset SHIZResourceImageIdMax
#define SHIZResourceSoundIdMax (SHIZResourceSoundIdOffset + SHIZResourceSoundMax)

#ifdef SHIZ_DEBUG
#define SHIZResourceIdMax SHIZResourceSoundIdMax
#endif

static bool _shiz_load_image_handler(int width, int height, int components,
                                     unsigned char * const data);

static bool _shiz_res_is_image(unsigned int const resource_id);
static bool _shiz_res_is_sound(unsigned int const resource_id);

static SHIZResourceType _shiz_res_get_type_from_id(unsigned int const resource_id);
static const int _shiz_res_get_index_from_id(unsigned int const resource_id, SHIZResourceType const type);

static unsigned int _shiz_res_next_id(SHIZResourceType const type, unsigned int *index);
static bool _shiz_res_is_index_free(unsigned int const resource_index, SHIZResourceType const type);

static const char * const _shiz_res_get_filename_ext(const char * const filename);

static SHIZResourceImage * _current_image_resource; // temporary pointer to the image being loaded

static SHIZResourceImage _images[SHIZResourceImageMax];
static SHIZResourceSound _sounds[SHIZResourceSoundMax];

#ifdef SHIZ_DEBUG
static SHIZResourceImage _font_resource;
#endif

SHIZResourceType const
shiz_res_get_type(const char * const filename) {
    SHIZResourceType resource_type = SHIZResourceTypeNotSupported;
    
    const char * extension = _shiz_res_get_filename_ext(filename);
    
    if (strcmp("png", extension) == 0) {
        resource_type = SHIZResourceTypeImage;
    } else if (strcmp("ogg", extension) == 0) {
        resource_type = SHIZResourceTypeSound;
    }
    
    return resource_type;
}

SHIZResourceImage
shiz_res_get_image(unsigned int const resource_id) {
    if (resource_id > SHIZResourceImageIdMax) {
#ifdef SHIZ_DEBUG
        if (resource_id == _font_resource.resource_id) {
            return _font_resource;
        }
#endif
        return SHIZResourceImageEmpty;
    }

    int const index = _shiz_res_get_index_from_id(resource_id, SHIZResourceTypeImage);

    if (index < 0) {
        return SHIZResourceImageEmpty;
    }

    return _images[index];
}

SHIZResourceSound
shiz_res_get_sound(unsigned int const resource_id) {
    if (resource_id > SHIZResourceSoundIdMax) {
        return SHIZResourceSoundEmpty;
    }

    int const index = _shiz_res_get_index_from_id(resource_id, SHIZResourceTypeSound);

    if (index < 0) {
        return SHIZResourceSoundEmpty;
    }

    return _sounds[index];
}

unsigned int
shiz_res_load(SHIZResourceType const type, const char * const filename) {
    if (type == SHIZResourceTypeNotSupported) {
        shiz_io_error("resource not loaded ('%s'); unsupported type (%s)",
                      filename, _shiz_res_get_filename_ext(filename));
        
        return SHIZResourceInvalid;
    }
    
    unsigned int resource_id = SHIZResourceInvalid;
    
    unsigned int expected_index;
    unsigned int const expected_id = _shiz_res_next_id(type, &expected_index);
    
    if (expected_id != SHIZResourceInvalid) {
        if (type == SHIZResourceTypeImage) {
            _current_image_resource = &_images[expected_index];
            
            if (shiz_io_load_image(filename, _shiz_load_image_handler)) {
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

bool
shiz_res_unload(unsigned int const resource_id) {
    if (resource_id == SHIZResourceInvalid) {
        return false;
    }

    SHIZResourceType const type = _shiz_res_get_type_from_id(resource_id);

    int const index = _shiz_res_get_index_from_id(resource_id, type);

    if (index < 0) {
        shiz_io_error("could not unload resource (%d); index out of bounds (%d)", resource_id, index);
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
            shiz_io_error("could not unload resource (%d); resource not found", resource_id);
        }
    }
    
    return false;
}

bool
shiz_res_unload_all() {
    bool something_failed = false;
    
    for (unsigned int image_resource_index = 0; image_resource_index < SHIZResourceImageMax; image_resource_index++) {
        unsigned int const resource_id = _images[image_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!shiz_res_unload(resource_id)) {
                something_failed = true;
            }
        }
    }
    
    for (unsigned int sound_resource_index = 0; sound_resource_index < SHIZResourceSoundMax; sound_resource_index++) {
        unsigned int const resource_id = _sounds[sound_resource_index].resource_id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!shiz_res_unload(resource_id)) {
                something_failed = true;
            }
        }
    }

#ifdef SHIZ_DEBUG
    if (_font_resource.resource_id != SHIZResourceInvalid) {
        if (!shiz_res_debug_unload_font()) {
            something_failed = true;
        }
    }
#endif
    
    return !something_failed;
}

static const int
_shiz_res_get_index_from_id(unsigned int const resource_id, SHIZResourceType const type) {
    if (type == SHIZResourceTypeImage) {
        return resource_id - SHIZResourceImageIdOffset;
    } else if (type == SHIZResourceTypeSound) {
        return resource_id - SHIZResourceSoundIdOffset;
    }
    
    return -1;
}

static SHIZResourceType
_shiz_res_get_type_from_id(unsigned int const resource_id) {
    if (_shiz_res_is_image(resource_id)) {
        return SHIZResourceTypeImage;
    } else if (_shiz_res_is_sound(resource_id)) {
        return SHIZResourceTypeSound;
    }
    
    return SHIZResourceTypeNotSupported;
}

static bool
_shiz_res_is_image(unsigned int const resource_id) {
    return resource_id < SHIZResourceImageIdMax;
}

static bool
_shiz_res_is_sound(unsigned int const resource_id) {
    return !_shiz_res_is_image(resource_id) && resource_id < SHIZResourceSoundIdMax;
}

static bool
_shiz_res_is_index_free(unsigned int const resource_index, SHIZResourceType const type) {
    if (type == SHIZResourceTypeImage) {
        return _images[resource_index].resource_id == SHIZResourceInvalid;
    } else if (type == SHIZResourceTypeSound) {
        return _sounds[resource_index].resource_id == SHIZResourceInvalid;
    }
    
    return false;
}

static unsigned int
_shiz_res_next_id(SHIZResourceType const type, unsigned int * index) {
    if (type == SHIZResourceTypeNotSupported) {
        return SHIZResourceInvalid;
    }
    
    unsigned int start_id = 0;
    unsigned int max_id = 0;
    
    if (type == SHIZResourceTypeImage) {
        start_id = SHIZResourceImageIdOffset;
        max_id = SHIZResourceImageIdMax;
    } else if (type == SHIZResourceTypeSound) {
        start_id = SHIZResourceSoundIdOffset;
        max_id = SHIZResourceSoundIdMax;
    }
    
    unsigned int next_index = 0;
    
    for (unsigned int next_id = start_id; next_id < max_id; next_id++) {
        next_index = next_id - start_id;
        
        if (_shiz_res_is_index_free(next_index, type)) {
            *index = next_index;
            
            return next_id;
        }
    }
    
    if (type == SHIZResourceTypeImage) {
        shiz_io_error("image limit reached (%d)", SHIZResourceImageMax);
    } else if (type == SHIZResourceTypeSound) {
        shiz_io_error("sound limit reached (%d)", SHIZResourceSoundMax);
    }
    
    return SHIZResourceInvalid;
}

static bool
_shiz_load_image_handler(int width, int height, int components, unsigned char * const data) {
    if (_current_image_resource == NULL) {
        return false;
    }

    _current_image_resource->width = width;
    _current_image_resource->height = height;

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

static const char * const
_shiz_res_get_filename_ext(const char * const filename) {
    const char * extension_index = strrchr(filename, '.');
    
    if (!extension_index || extension_index == filename) {
        return NULL;
    }
    
    return extension_index + 1; // extension without the '.'
}

#ifdef SHIZ_DEBUG
void
shiz_res_debug_print_resources() {
    printf("  IDX  ID  RESOURCE\n");
    printf("  -----------------\n");
    for (unsigned int image_resource_index = 0; image_resource_index < SHIZResourceImageMax; image_resource_index++) {
        unsigned int resource_id = _images[image_resource_index].resource_id;
        
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
    for (unsigned int sound_resource_index = 0; sound_resource_index < SHIZResourceSoundMax; sound_resource_index++) {
        unsigned int resource_id = _sounds[sound_resource_index].resource_id;
        
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
shiz_res_debug_load_font(const unsigned char * const buffer, unsigned int const length) {
    if (_font_resource.resource_id != SHIZResourceInvalid) {
        return false;
    }

    _current_image_resource = &_font_resource;

    if (shiz_io_load_image_data(buffer, length, _shiz_load_image_handler)) {
        _font_resource.resource_id = SHIZResourceIdMax + 1; // offset by 1 to ensure the id won't be occupied by any other resource
    }

    _current_image_resource = NULL;

    return true;
}

bool
shiz_res_debug_unload_font() {
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

unsigned int
shiz_res_debug_get_font() {
    return _font_resource.resource_id;
}
#endif
