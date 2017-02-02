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

#ifndef internal_h
#define internal_h

#define SHIZEN_VERSION_MAJOR 0
#define SHIZEN_VERSION_MINOR 15
#define SHIZEN_VERSION_PATCH 0

#define SHIZEN_VERSION_NAME "ALPHA"

#define GLFW_INCLUDE_NONE

#include <GL/gl3w.h>

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wdocumentation"
#endif

#include <GLFW/glfw3.h>

#if defined(__clang__)
 #pragma clang pop
#endif

#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <SHIZEN/type.h>

#ifdef SHIZ_DEBUG
 #include "debug.h"
#endif

#define SHIZEpsilon (1.0 / 1024)

typedef struct SHIZTimeLine {
    double time;
    double time_step;
    double scale;
} SHIZTimeLine;

typedef struct SHIZViewport {
    SHIZSize framebuffer;
    SHIZSize screen;
    float scale; // framebuffer pixel scale; i.e. retina @2x framebuffer at 640 => actually 1280
    SHIZSize offset; // offset if letter/pillarboxing is enabled
    bool is_fullscreen;
} SHIZViewport;

SHIZSize _shiz_get_preferred_screen_size(void);

void _shiz_present_frame(void);

static inline void
_shiz_str_to_upper(char * string)
{
    char * ptr;

    for (ptr = string; *ptr; ptr++) {
        *ptr = (char)toupper(*ptr);
    }
}

static inline bool
_shiz_fequal(float const a, float const b)
{
    return (fabs(b - a) < SHIZEpsilon);
}

static inline float
_shiz_lerp(float const a, float const b, float const t)
{
    return a * (1.0f - t) + b * t;
}

static inline int
_shiz_random_int_range(int const min, int const max)
{
    return min + (rand() % (int)((max + 1) - min));
}

static inline float
_shiz_random_float(void)
{
    return rand() / (RAND_MAX + 1.0f);
}

static inline float
_shiz_random_float_range(float const min, float const max)
{
    return _shiz_lerp(min, max, _shiz_random_float());
}

static inline float
_shiz_layer_get_z(SHIZLayer const layer)
{
    float const value = layer.layer + layer.depth;
    
    float const min = SHIZLayerMin + SHIZLayerDepthMin;
    float const max = SHIZLayerMax + SHIZLayerDepthMax;
    
    return (float)(value - min) / (max - min);
}

#endif // internal_h
