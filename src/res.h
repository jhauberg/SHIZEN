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

typedef enum SHIZResourceType SHIZResourceType;

typedef struct SHIZResourceImage SHIZResourceImage;
typedef struct SHIZResourceSound SHIZResourceSound;

enum SHIZResourceType {
    SHIZResourceTypeNotSupported,
    SHIZResourceTypeImage,
    SHIZResourceTypeSound
};

struct SHIZResourceImage {
    uint id;
    uint width;
    uint height;
    GLuint texture_id;
    const char *filename;
};

struct SHIZResourceSound {
    uint id;
    const char *filename;
};

static SHIZResourceImage const SHIZResourceImageEmpty = { 0, 0, 0, 0, NULL };
static SHIZResourceSound const SHIZResourceSoundEmpty = { 0, NULL };

uint shiz_res_load(SHIZResourceType const type, const char * const filename);

bool shiz_res_unload(uint const resource_id);
bool shiz_res_unload_all(void);

const SHIZResourceType shiz_res_get_type(const char * const filename);

SHIZResourceImage shiz_res_get_image(uint const resource_id);
SHIZResourceSound shiz_res_get_sound(uint const resource_id);

#ifdef SHIZ_DEBUG
void shiz_res_debug_print_resources(void);

bool shiz_res_debug_load_font(void);
bool shiz_res_debug_unload_font(void);

uint shiz_res_debug_get_font(void);
#endif

#endif // res_h
