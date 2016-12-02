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

typedef enum {
    SHIZResourceTypeNotSupported,
    SHIZResourceTypeImage,
    SHIZResourceTypeSound
} SHIZResourceType;

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

static SHIZResourceImage const SHIZResourceImageEmpty = { 0, 0, 0, 0, NULL };
static SHIZResourceSound const SHIZResourceSoundEmpty = { 0, NULL };

// load a resource and return its id - returns invalid id (0) if loading failed
uint shiz_res_load(SHIZResourceType const type, const char *filename);
// unload a resource - returns true if unloading succeeded, false otherwise
bool shiz_res_unload(uint const resource_id);
bool shiz_res_unload_all(void);

const SHIZResourceType shiz_res_get_type(const char *filename);

SHIZResourceImage shiz_res_get_image(uint const resource_id);
SHIZResourceSound shiz_res_get_sound(uint const resource_id);

#ifdef DEBUG
void shiz_res_debug_print_resources(void);
#endif

#endif // res_h
