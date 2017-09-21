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
    char const * filename;
    GLuint texture_id;
    u16 width;
    u16 height;
    u8 resource_id;
} SHIZResourceImage;

typedef struct SHIZResourceSound {
    char const * filename;
    u8 resource_id;
} SHIZResourceSound;

extern SHIZResourceImage const SHIZResourceImageEmpty;
extern SHIZResourceSound const SHIZResourceSoundEmpty;

extern u8 const SHIZResourceInvalid;

u8
z_res__load(SHIZResourceType type,
            char const * filename);

bool
z_res__unload(u8 resource_id);

bool
z_res__unload_all(void);

SHIZResourceType const
z_res__type(char const * const filename);

SHIZResourceImage
z_res__image(u8 resource_id);

SHIZResourceSound
z_res__sound(u8 resource_id);

#endif // res_h
