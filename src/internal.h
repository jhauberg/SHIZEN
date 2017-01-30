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
#define SHIZEN_VERSION_MINOR 14
#define SHIZEN_VERSION_PATCH 4

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

SHIZSize _shiz_get_preferred_screen_size(void);

void _shiz_present_frame(void);

static inline unsigned int const _shiz_get_char_size(char const character) {
    int bits = 7;
    unsigned int size = 0;
    
    while (bits >= 0) {
        if (!((character >> bits) & 1)) {
            break;
        }
        
        size += 1;
        bits -= 1;
    }
    
    if (size == 0) {
        size = sizeof(char);
    }
    
    return size; // in bytes
}

static inline void _shiz_str_to_upper(char * string) {
    char * ptr;

    for (ptr = string; *ptr != '\0'; ptr++) {
        *ptr = (char)toupper(*ptr);
    }
}

static inline bool _shiz_fequal(float const a, float const b) {
    return (fabs(b - a) < SHIZEpsilon);
}

static inline float _shiz_lerp(float const a, float const b, float const t) {
    return a * (1.0f - t) + b * t;
}

static inline int _shiz_random_int_range(int const min, int const max) {
    return min + (rand() % (int)((max + 1) - min));
}

static inline float _shiz_random_float(void) {
    return rand() / (RAND_MAX + 1.0f);
}

static inline float _shiz_random_float_range(float const min, float const max) {
    return _shiz_lerp(min, max, _shiz_random_float());
}

static inline float _shiz_layer_get_z(SHIZLayer const layer) {
    float const value = layer.layer + layer.depth;
    
    float const min = SHIZLayerMin + SHIZLayerDepthMin;
    float const max = SHIZLayerMax + SHIZLayerDepthMax;
    
    return (float)(value - min) / (max - min);
}

#endif // internal_h
