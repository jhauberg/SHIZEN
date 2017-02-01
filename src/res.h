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

#include "internal.h"

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

unsigned int shiz_res_load(SHIZResourceType const type, const char * const filename);

bool shiz_res_unload(unsigned int const resource_id);
bool shiz_res_unload_all(void);

SHIZResourceType const shiz_res_get_type(const char * const filename);

SHIZResourceImage shiz_res_get_image(unsigned int const resource_id);
SHIZResourceSound shiz_res_get_sound(unsigned int const resource_id);

#ifdef SHIZ_DEBUG
void shiz_res_debug_print_resources(void);

bool shiz_res_debug_load_font(const unsigned char * const buffer, unsigned int const length);
bool shiz_res_debug_unload_font(void);

unsigned int shiz_res_debug_get_font(void);
#endif

#endif // res_h
