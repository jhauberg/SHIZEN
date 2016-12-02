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

typedef struct {
    uint id;
    uint width;
    uint height;
    GLuint texture_id;
    const char *filename;
} SHIZResourceImage;

typedef struct {
    uint id;
    const char *filename;
} SHIZResourceSound;

static bool _shiz_load_image_handler(int width, int height, int components, unsigned char* data);
static int _shiz_load_image_index = -1; /* the temporary index of the image being loaded in the handler implementation; any better solution for providing this index? */

static bool _shiz_res_is_image(uint const resource_id);
static bool _shiz_res_is_sound(uint const resource_id);

static uint _shiz_res_next_id(SHIZResourceType const type, uint *index);
static bool _shiz_res_is_index_free(uint const resource_index, SHIZResourceType const type);

static const char* _shiz_res_get_filename_ext(const char *filename);

static uint const max_images = 32;
static uint const max_sounds = 16;

static uint const invalid_resource_id = 0;

static uint const image_resource_id_offset = 1; /* offset by 1 to skip invalid resource id (0); index 0 still used */
static uint const image_resource_id_max = max_images + image_resource_id_offset;

static uint const sound_resource_id_offset = image_resource_id_max;
static uint const sound_resource_id_max = max_sounds + sound_resource_id_offset;

static SHIZResourceImage images[max_images];
static SHIZResourceSound sounds[max_sounds];

const SHIZResourceType shiz_res_get_type(const char *filename) {
    SHIZResourceType resource_type = SHIZResourceTypeNotSupported;
    
    const char *extension = _shiz_res_get_filename_ext(filename);
    
    if (strcmp("png", extension) == 0) {
        resource_type = SHIZResourceTypeImage;
    } else if (strcmp("ogg", extension) == 0) {
        resource_type = SHIZResourceTypeSound;
    }
    
    return resource_type;
}

uint shiz_res_load(SHIZResourceType const type, const char *filename) {
    if (type == SHIZResourceTypeNotSupported) {
        shiz_io_error("resource not loaded ('%s'); unsupported type (%s)",
                      filename, _shiz_res_get_filename_ext(filename));
        
        return invalid_resource_id;
    }
    
    uint resource_id = invalid_resource_id;
    
    uint expected_index;
    uint const expected_id = _shiz_res_next_id(type, &expected_index);
    
    if (expected_id != invalid_resource_id) {
        if (type == SHIZResourceTypeImage) {
            _shiz_load_image_index = expected_index;
            
            if (shiz_io_load_image(filename, _shiz_load_image_handler)) {
                images[expected_index].id = expected_id;
                images[expected_index].filename = filename;
                
                resource_id = expected_id;
            }
            
            _shiz_load_image_index = -1;
        } else if (type == SHIZResourceTypeSound) {
            // todo: actual loading left blank for future implementation
            
            sounds[expected_index].id = expected_id;
            sounds[expected_index].filename = filename;
            
            resource_id = expected_id;
        }
    }
    
    return resource_id;
}

bool shiz_res_unload(uint const resource_id) {
    if (resource_id == invalid_resource_id) {
        return false;
    }
    
    if (_shiz_res_is_image(resource_id)) {
        uint const index = resource_id - image_resource_id_offset;
        
        glDeleteTextures(1, &images[index].texture_id);
        
        images[index].width = 0;
        images[index].height = 0;
        images[index].id = invalid_resource_id;
        images[index].filename = NULL;
        images[index].texture_id = 0;
        
        return true;
    } else if (_shiz_res_is_sound(resource_id)) {
        uint const index = resource_id - sound_resource_id_offset;
        
        // todo: actual unloading left blank for future implementation
        
        sounds[index].id = invalid_resource_id;
        
        return true;
    }
    
    return false;
}

bool shiz_res_unload_all() {
    bool something_failed = false;
    
    for (uint image_resource_index = 0; image_resource_index < max_images; image_resource_index++) {
        uint resource_id = images[image_resource_index].id;
        
        if (resource_id != invalid_resource_id) {
            if (!shiz_res_unload(resource_id)) {
                something_failed = true;
                
                shiz_io_error("failed unloading image resource (%d)", resource_id);
            }
        }
    }
    
    for (uint sound_resource_index = 0; sound_resource_index < max_sounds; sound_resource_index++) {
        uint resource_id = sounds[sound_resource_index].id;
        
        if (resource_id != invalid_resource_id) {
            if (!shiz_res_unload(resource_id)) {
                something_failed = true;
                
                shiz_io_error("failed unloading sound resource (%d)", resource_id);
            }
        }
    }
    
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
        return images[resource_index].id == invalid_resource_id;
    } else if (type == SHIZResourceTypeSound) {
        return sounds[resource_index].id == invalid_resource_id;
    }
    
    return false;
}

static uint _shiz_res_next_id(SHIZResourceType const type, uint *index) {
    if (type == SHIZResourceTypeNotSupported) {
        return invalid_resource_id;
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
    
    return invalid_resource_id;
}

static bool _shiz_load_image_handler(int width, int height, int components, unsigned char* data) {
    if (_shiz_load_image_index == -1) {
        return false;
    }
    
    images[_shiz_load_image_index].width = width;
    images[_shiz_load_image_index].height = height;
    
    glGenTextures(1, &images[_shiz_load_image_index].texture_id);
    glBindTexture(GL_TEXTURE_2D, images[_shiz_load_image_index].texture_id); {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        if (components == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (components == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

static const char* _shiz_res_get_filename_ext(const char *filename) {
    const char *extension_index = strrchr(filename, '.');
    
    if (!extension_index || extension_index == filename) {
        return NULL;
    }
    
    return extension_index + 1; // extension without the '.'
}

#ifdef DEBUG
void shiz_res_debug_print_resources() {
    printf("\n");
    printf("  IDX  ID  RESOURCE\n");
    printf("  -----------------\n");
    for (uint image_resource_index = 0; image_resource_index < max_images; image_resource_index++) {
        uint resource_id = images[image_resource_index].id;
        
        if (resource_id != invalid_resource_id) {
            printf("  %02d: (%02d) %s %dx%d\n",
                   image_resource_index, resource_id,
                   images[image_resource_index].filename,
                   images[image_resource_index].width,
                   images[image_resource_index].height);
        } else {
            printf("  %02d: (%02d) ---\n", image_resource_index, resource_id);
        }
    }
}
#endif
