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

#ifndef res_h
#define res_h

#include <stdio.h>
#include <stdbool.h>

#include "internal_type.h"

typedef enum SHIZResourceType {
    SHIZResourceTypeNotSupported,
    SHIZResourceTypeImage,
    SHIZResourceTypeSound
} SHIZResourceType;

typedef struct SHIZResourceImage {
    GLuint texture_id;
    unsigned int resource_id;
    unsigned int width;
    unsigned int height;
    const char * filename;
} SHIZResourceImage;

typedef struct SHIZResourceSound {
    unsigned int resource_id;
    const char * filename;
} SHIZResourceSound;

extern SHIZResourceImage const SHIZResourceImageEmpty;
extern SHIZResourceSound const SHIZResourceSoundEmpty;

extern unsigned int const SHIZResourceInvalid;

unsigned int z_res__load(SHIZResourceType type, char const * filename);

bool z_res__unload(unsigned int resource_id);
bool z_res__unload_all(void);

SHIZResourceType const z_res__type(char const * const filename);

SHIZResourceImage z_res__image(unsigned int resource_id);
SHIZResourceSound z_res__sound(unsigned int resource_id);

#ifdef SHIZ_DEBUG
void shiz_res_debug_print_resources(void);

bool shiz_res_debug_load_font(unsigned char const * buffer, unsigned int length);
bool shiz_res_debug_unload_font(void);

unsigned int shiz_res_debug_get_font(void);
#endif

#endif // res_h
