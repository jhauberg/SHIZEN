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
#define SHIZEN_VERSION_MINOR 18
#define SHIZEN_VERSION_PATCH 2

#define SHIZEN_VERSION_NAME "ALPHA"

// use gl3w to load opengl headers
#include <GL/gl3w.h>
// which means GLFW should not do that
#define GLFW_INCLUDE_NONE

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wdocumentation"
 #pragma clang diagnostic ignored "-Wconversion"
#endif

#include <GLFW/glfw3.h>

#include <linmath.h>

#if defined(__clang__)
 #pragma clang pop
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <SHIZEN/ztype.h>

#ifdef SHIZ_DEBUG
 #include "debug.h"
#endif

#define SHIZEpsilon (1.0 / 1024)

typedef struct SHIZGraphicsContext {
    /** A reference to the current window */
    GLFWwindow * window;
    /** The operating resolution; the display may be boxed if necessary */
    SHIZSize native_size;
    /** The actual display size; essentially operating resolution ⨉ pixel size */
    SHIZSize display_size;
    /** The number of screen updates between buffer swaps */
    int swap_interval;
    /** The size of each pixel; must be higher than 0 */
    unsigned int pixel_size;
    /** Determines whether the context has been initialized */
    bool is_initialized;
    /** Determines whether the context has focus */
    bool is_focused;
    /** Determines whether a shutdown should be initiated */
    bool should_finish;
} SHIZGraphicsContext;

typedef struct SHIZTimeline {
    double time;
    double time_step;
    double scale;
} SHIZTimeline;

typedef struct SHIZRenderObject {
    GLuint program;
    GLuint vbo;
    GLuint vao;
} SHIZRenderObject;

typedef struct SHIZVertexPositionColor {
    SHIZVector3 position;
    SHIZColor color;
} SHIZVertexPositionColor;

typedef struct SHIZVertexPositionTexture {
    SHIZVector3 position;
    SHIZVector2 texture_coord;
} SHIZVertexPositionTexture;

typedef struct SHIZVertexPositionColorTexture {
    SHIZVector3 position;
    SHIZColor color;
    SHIZVector2 texture_coord;
    SHIZVector2 texture_coord_min;
    SHIZVector2 texture_coord_max;
} SHIZVertexPositionColorTexture;

void _shiz_present_frame(void);

static inline
void
_shiz_str_to_upper(char * string)
{
    char * ptr;

    for (ptr = string; *ptr; ptr++) {
        *ptr = (char)toupper(*ptr);
    }
}

/**
 * @brief Determine whether two floats are approximately equal.
 */
static inline
bool
_shiz_fequal(float const a, float const b)
{
    return (fabs(b - a) < SHIZEpsilon);
}

static inline
float
_shiz_lerp(float const a, float const b, float const t)
{
    return a * (1.0f - t) + b * t;
}

static inline
int
_shiz_random_int_range(int const min, int const max)
{
    return min + (rand() % (int)((max + 1) - min));
}

static inline
float
_shiz_random_float(void)
{
    return rand() / (RAND_MAX + 1.0f);
}

static inline
float
_shiz_random_float_range(float const min, float const max)
{
    return _shiz_lerp(min, max, _shiz_random_float());
}

static inline
float
_shiz_layer_get_z(SHIZLayer const layer)
{
    float const value = layer.layer + layer.depth;
    
    float const min = SHIZLayerMin + SHIZLayerDepthMin;
    float const max = SHIZLayerMax + SHIZLayerDepthMax;
    
    return (float)(value - min) / (max - min);
}

#endif // internal_h
