////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2017 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#ifndef viewport_h
#define viewport_h

#include <SHIZEN/ztype.h>

typedef struct SHIZViewport {
    SHIZSize framebuffer;
    SHIZSize resolution;
    float scale; // framebuffer pixel scale; i.e. retina @2x framebuffer at 640 => actually 1280
    bool is_fullscreen;
} SHIZViewport;

typedef enum SHIZViewportMode {
    SHIZViewportModeNormal,
    SHIZViewportModeLetterbox,
    SHIZViewportModePillarbox
} SHIZViewportMode;

extern SHIZViewport const SHIZViewportDefault;

void shiz_set_viewport(SHIZViewport viewport);

SHIZViewport shiz_get_viewport(void);
SHIZSize shiz_get_viewport_offset(void);

#endif // viewport_h
