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

#ifndef internal_type_h
#define internal_type_h

// use gl3w to load opengl headers
#include <GL/gl3w.h>
// which means GLFW should not do that
#define GLFW_INCLUDE_NONE

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wdocumentation"
#endif

#include <GLFW/glfw3.h>

#include <linmath.h>

#if defined(__clang__)
 #pragma clang diagnostic pop
#endif

#include <SHIZEN/ztype.h>

#define PIXEL(x) floorf(x)
#define HALF_PIXEL 0.5f

typedef struct SHIZGraphicsContext {
    /** A reference to the current window */
    GLFWwindow * window;
    /** The operating resolution; the display may be boxed if necessary */
    SHIZSize native_size;
    /** The actual display size; essentially operating resolution ⨉ pixel size */
    SHIZSize display_size;
    /** The number of screen updates between buffer swaps */
    u8 swap_interval;
    /** The size of each pixel; must be higher than 0 */
    u8 pixel_size;
    /** Determines whether the context has been initialized */
    bool is_initialized;
    /** Determines whether the context has focus */
    bool is_focused;
    /** Determines whether the context is running in fullscreen */
    bool is_fullscreen;
    /** Determines whether a shutdown should be initiated */
    bool should_finish;
    u8 _pad[2];
} SHIZGraphicsContext;

typedef struct SHIZTimeline {
    f64 time;
    f64 time_step;
    f64 scale;
} SHIZTimeline;

typedef struct SHIZRenderObject {
    GLuint program;
    GLuint vbo;
    GLuint vao;
} SHIZRenderObject;

typedef struct SHIZVector3 {
    f32 x, y, z;
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
SHIZVector3Make(f32 const x, f32 const y, f32 const z)
{
    SHIZVector3 const vector = {
        x, y, z
    };
    
    return vector;
}

f32 const
z_layer__get_z(SHIZLayer layer);

#endif /* internal_type_h */
