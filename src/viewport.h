#pragma once

#include <SHIZEN/ztype.h> // SHIZSize, SHIZRect

typedef struct SHIZViewport {
    SHIZSize framebuffer;
    SHIZSize resolution;
    float scale; // framebuffer pixel scale; i.e. retina @2x framebuffer at 640 => actually 1280
} SHIZViewport;

typedef enum SHIZViewportMode {
    SHIZViewportModeNormal,
    SHIZViewportModeLetterbox,
    SHIZViewportModePillarbox
} SHIZViewportMode;

extern SHIZViewport const SHIZViewportDefault;

void z_viewport__set(SHIZViewport viewport);

SHIZViewport z_viewport__get(void);
SHIZRect z_viewport__get_clip(void);
