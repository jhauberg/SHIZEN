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
#include "res.font.h"
#include "io.h"

uint const SHIZResourceInvalid = 0;

static bool _shiz_load_image_handler(int width, int height, int components,
                                     unsigned char * const data);

static SHIZResourceImage *_shiz_load_image_resource; // temporary pointer to the image being loaded

static bool _shiz_res_is_image(uint const resource_id);
static bool _shiz_res_is_sound(uint const resource_id);

static SHIZResourceType _shiz_res_get_type_from_id(uint const resource_id);
static const int _shiz_res_get_index_from_id(uint const resource_id, SHIZResourceType const type);

static uint _shiz_res_next_id(SHIZResourceType const type, uint *index);
static bool _shiz_res_is_index_free(uint const resource_index, SHIZResourceType const type);

static const char* _shiz_res_get_filename_ext(const char * const filename);

static uint const max_images = 16;
static uint const max_sounds = 8;

static uint const image_resource_id_offset = 1; /* offset by 1 to skip invalid resource id (0); index 0 still used */
static uint const image_resource_id_max = max_images + image_resource_id_offset;

static uint const sound_resource_id_offset = image_resource_id_max;
static uint const sound_resource_id_max = max_sounds + sound_resource_id_offset;

#ifdef SHIZ_DEBUG
static uint const resource_id_max = sound_resource_id_max;
#endif

static SHIZResourceImage images[max_images];
static SHIZResourceSound sounds[max_sounds];

#ifdef SHIZ_DEBUG
static SHIZResourceImage debug_font_resource;
#endif

const SHIZResourceType shiz_res_get_type(const char * const filename) {
    SHIZResourceType resource_type = SHIZResourceTypeNotSupported;
    
    const char *extension = _shiz_res_get_filename_ext(filename);
    
    if (strcmp("png", extension) == 0) {
        resource_type = SHIZResourceTypeImage;
    } else if (strcmp("ogg", extension) == 0) {
        resource_type = SHIZResourceTypeSound;
    }
    
    return resource_type;
}

SHIZResourceImage shiz_res_get_image(uint const resource_id) {
    if (resource_id > image_resource_id_max) {
#ifdef SHIZ_DEBUG
        if (resource_id == debug_font_resource.id) {
            return debug_font_resource;
        }
#endif
        return SHIZResourceImageEmpty;
    }

    int const index = _shiz_res_get_index_from_id(resource_id, SHIZResourceTypeImage);

    if (index < 0) {
        return SHIZResourceImageEmpty;
    }

    return images[index];
}

SHIZResourceSound shiz_res_get_sound(uint const resource_id) {
    if (resource_id > image_resource_id_max) {
        return SHIZResourceSoundEmpty;
    }

    int const index = _shiz_res_get_index_from_id(resource_id, SHIZResourceTypeSound);

    if (index < 0) {
        return SHIZResourceSoundEmpty;
    }

    return sounds[index];
}

uint shiz_res_load(SHIZResourceType const type, const char * const filename) {
    if (type == SHIZResourceTypeNotSupported) {
        shiz_io_error("resource not loaded ('%s'); unsupported type (%s)",
                      filename, _shiz_res_get_filename_ext(filename));
        
        return SHIZResourceInvalid;
    }
    
    uint resource_id = SHIZResourceInvalid;
    
    uint expected_index;
    uint const expected_id = _shiz_res_next_id(type, &expected_index);
    
    if (expected_id != SHIZResourceInvalid) {
        if (type == SHIZResourceTypeImage) {
            _shiz_load_image_resource = &images[expected_index];
            
            if (shiz_io_load_image(filename, _shiz_load_image_handler)) {
                _shiz_load_image_resource->id = expected_id;
                _shiz_load_image_resource->filename = filename;
                
                resource_id = expected_id;
            }

            _shiz_load_image_resource = NULL;
        } else if (type == SHIZResourceTypeSound) {
            // todo: actual loading left blank for future implementation
            
            sounds[expected_index].id = expected_id;
            sounds[expected_index].filename = filename;
            
            resource_id = expected_id;
        }
    }
    
    return resource_id;
}

static const int _shiz_res_get_index_from_id(uint const resource_id, SHIZResourceType const type) {
    if (type == SHIZResourceTypeImage) {
        return resource_id - image_resource_id_offset;
    } else if (type == SHIZResourceTypeSound) {
        return resource_id - sound_resource_id_offset;
    }

    return -1;
}

static SHIZResourceType _shiz_res_get_type_from_id(uint const resource_id) {
    if (_shiz_res_is_image(resource_id)) {
        return SHIZResourceTypeImage;
    } else if (_shiz_res_is_sound(resource_id)) {
        return SHIZResourceTypeSound;
    }

    return SHIZResourceTypeNotSupported;
}

bool shiz_res_unload(uint const resource_id) {
    if (resource_id == SHIZResourceInvalid) {
        return false;
    }

    SHIZResourceType type = _shiz_res_get_type_from_id(resource_id);

    int const index = _shiz_res_get_index_from_id(resource_id, type);

    if (index < 0) {
        shiz_io_error("could not unload resource (%d); index out of bounds (%d)", resource_id, index);
    } else {
        if (type == SHIZResourceTypeImage) {
            glDeleteTextures(1, &images[index].texture_id);
            
            images[index].width = 0;
            images[index].height = 0;
            images[index].id = SHIZResourceInvalid;
            images[index].filename = NULL;
            images[index].texture_id = 0;
            
            return true;
        } else if (type == SHIZResourceTypeSound) {
            // todo: actual unloading left blank for future implementation
            
            sounds[index].id = SHIZResourceInvalid;
            
            return true;
        } else {
            shiz_io_error("could not unload resource (%d); resource not found", resource_id);
        }
    }
    
    return false;
}

bool shiz_res_unload_all() {
    bool something_failed = false;
    
    for (uint image_resource_index = 0; image_resource_index < max_images; image_resource_index++) {
        uint resource_id = images[image_resource_index].id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!shiz_res_unload(resource_id)) {
                something_failed = true;
            }
        }
    }
    
    for (uint sound_resource_index = 0; sound_resource_index < max_sounds; sound_resource_index++) {
        uint resource_id = sounds[sound_resource_index].id;
        
        if (resource_id != SHIZResourceInvalid) {
            if (!shiz_res_unload(resource_id)) {
                something_failed = true;
            }
        }
    }

#ifdef SHIZ_DEBUG
    if (!shiz_res_debug_unload_font()) {
        something_failed = true;
    }
#endif
    
    return !something_failed;
}

static bool _shiz_res_is_image(uint const resource_id) {
    return resource_id < image_resource_id_max;
}

static bool _shiz_res_is_sound(uint const resource_id) {
    return !_shiz_res_is_image(resource_id) && resource_id < sound_resource_id_max;
}

static bool _shiz_res_is_index_free(uint const resource_index, SHIZResourceType const type) {
    if (type == SHIZResourceTypeImage) {
        return images[resource_index].id == SHIZResourceInvalid;
    } else if (type == SHIZResourceTypeSound) {
        return sounds[resource_index].id == SHIZResourceInvalid;
    }
    
    return false;
}

static uint _shiz_res_next_id(SHIZResourceType const type, uint * index) {
    if (type == SHIZResourceTypeNotSupported) {
        return SHIZResourceInvalid;
    }
    
    uint start_id = 0;
    uint max_id = 0;
    
    if (type == SHIZResourceTypeImage) {
        start_id = image_resource_id_offset;
        max_id = image_resource_id_max;
    } else if (type == SHIZResourceTypeSound) {
        start_id = sound_resource_id_offset;
        max_id = sound_resource_id_max;
    }
    
    uint next_index = 0;
    
    for (uint next_id = start_id; next_id < max_id; next_id++) {
        next_index = next_id - start_id;
        
        if (_shiz_res_is_index_free(next_index, type)) {
            *index = next_index;
            
            return next_id;
        }
    }
    
    if (type == SHIZResourceTypeImage) {
        shiz_io_error("image limit reached (%d)", max_images);
    } else if (type == SHIZResourceTypeSound) {
        shiz_io_error("sound limit reached (%d)", max_sounds);
    }
    
    return SHIZResourceInvalid;
}

static bool _shiz_load_image_handler(int width, int height, int components,
                                     unsigned char * const data) {
    if (_shiz_load_image_resource == NULL) {
        return false;
    }

    _shiz_load_image_resource->width = width;
    _shiz_load_image_resource->height = height;

    glGenTextures(1, &_shiz_load_image_resource->texture_id);
    glBindTexture(GL_TEXTURE_2D, _shiz_load_image_resource->texture_id); {
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

static const char* _shiz_res_get_filename_ext(const char * const filename) {
    const char *extension_index = strrchr(filename, '.');
    
    if (!extension_index || extension_index == filename) {
        return NULL;
    }
    
    return extension_index + 1; // extension without the '.'
}

#ifdef SHIZ_DEBUG
void shiz_res_debug_print_resources() {
    printf("  IDX  ID  RESOURCE\n");
    printf("  -----------------\n");
    for (uint image_resource_index = 0; image_resource_index < max_images; image_resource_index++) {
        uint resource_id = images[image_resource_index].id;
        
        if (resource_id != SHIZResourceInvalid) {
            printf("i %02d: [%02d] %s (%dx%d)\n",
                   image_resource_index, resource_id,
                   images[image_resource_index].filename,
                   images[image_resource_index].width,
                   images[image_resource_index].height);
        } else {
            printf("i %02d: [%02d] ---\n", image_resource_index, resource_id);
        }
    }
    for (uint sound_resource_index = 0; sound_resource_index < max_sounds; sound_resource_index++) {
        uint resource_id = sounds[sound_resource_index].id;
        
        if (resource_id != SHIZResourceInvalid) {
            printf("s %02d: [%02d] %s\n",
                   sound_resource_index, resource_id,
                   sounds[sound_resource_index].filename);
        } else {
            printf("s %02d: [%02d] ---\n", sound_resource_index, resource_id);
        }
    }
}

bool shiz_res_debug_load_font() {
    if (debug_font_resource.id != SHIZResourceInvalid) {
        return false;
    }

    _shiz_load_image_resource = &debug_font_resource;

    if (shiz_io_load_image_data(debug_font_data, debug_font_data_length, _shiz_load_image_handler)) {
        debug_font_resource.id = resource_id_max + 1;
    }

    _shiz_load_image_resource = NULL;

    return true;
}

bool shiz_res_debug_unload_font() {
    glDeleteTextures(1, &debug_font_resource.texture_id);

    debug_font_resource.width = 0;
    debug_font_resource.height = 0;
    debug_font_resource.id = SHIZResourceInvalid;
    debug_font_resource.filename = NULL;
    debug_font_resource.texture_id = 0;

    return true;
}

uint shiz_res_debug_get_font() {
    return debug_font_resource.id;
}
#endif
