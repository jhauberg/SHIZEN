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

#pragma once

#define SHIZEN_VERSION_MAJOR 0
#define SHIZEN_VERSION_MINOR 21
#define SHIZEN_VERSION_PATCH 1

#define SHIZEN_VERSION_NAME "ALPHA"

#include <ctype.h> // toupper

// use gl3w to load opengl headers
#include <gl3w/GL/gl3w.h>
// which means GLFW should not do that
#define GLFW_INCLUDE_NONE

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wdocumentation"
#endif

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <GLFW/glfw3.h>

#include <linmath/linmath.h>

#if defined(__clang__)
 #pragma clang diagnostic pop
#endif

#include <SHIZEN/ztype.h>

/**
 * This toggle determines how pixel coordinates are transformed before being
 * sent to the GPU, and has an effect on how sprites and shapes are
 * rasterized when rendered.
 *
 * If enabled, all pixel coordinates will be floored to their integral value;
 * e.g. '12.5' becomes '12.0', providing precise pixel rasterization.
 *
 * Generally, enabling this option should result in precise renderings,
 * but might also make movement look less fluid/smooth.
 *
 * Turning it off, however, may cause unexpected rasterization artifacts and
 * less precise renderings, but will also make movement look more fluid.
 *
 * So there's a trade-off to consider between both options.
 */
#define PREFER_INTEGRAL_PIXELS 0

#if PREFER_INTEGRAL_PIXELS
 #define PIXEL(x) floorf(x)
#else
 #define PIXEL(x) x
#endif

#define HALF_PIXEL 0.5f

typedef struct SHIZGraphicsContext {
    /** A reference to the current window */
    GLFWwindow * window;
    /** The operating resolution; the display may be boxed if necessary */
    SHIZSize native_size;
    /** The actual display size; essentially operating resolution ⨉ pixel size */
    SHIZSize display_size;
    /** The number of screen updates between buffer swaps */
    uint8_t swap_interval;
    /** The size of each pixel; must be higher than 0 */
    uint8_t pixel_size;
    /** Determines whether the context has been initialized */
    bool is_initialized;
    /** Determines whether the context has focus */
    bool is_focused;
    /** Determines whether the context is running in fullscreen */
    bool is_fullscreen;
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

typedef struct SHIZVector3 {
    float x, y, z;
} SHIZVector3;

extern SHIZVector3 const SHIZVector3Zero;
extern SHIZVector3 const SHIZVector3One;

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

static inline
SHIZVector3 const
SHIZVector3Make(float const x, float const y, float const z)
{
    SHIZVector3 const vector = {
        x, y, z
    };
    
    return vector;
}

void z_engine__present_frame(void);

#ifdef SHIZ_DEBUG
double z_time__get_lag(void);
#endif

static inline
void
z_str_to_upper(char * string)
{
    char * ptr;

    for (ptr = string; *ptr; ptr++) {
        *ptr = (char)toupper(*ptr);
    }
}

float const z_layer__get_z(SHIZLayer layer);
