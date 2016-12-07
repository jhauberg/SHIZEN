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

#include "type.h"

typedef struct {
    bool is_initialized;
    bool is_focused;

    bool should_finish;
    
    SHIZSize preferred_screen_size;
    
    GLFWwindow *window;
} SHIZGraphicsContext;

typedef enum {
    SHIZViewportModeNormal,
    SHIZViewportModeLetterbox,
    SHIZViewportModePillarbox
} SHIZViewportMode;

typedef struct {
    SHIZSize framebuffer;
    SHIZSize screen;
    float scale; // framebuffer pixel scale; i.e. retina @2x framebuffer at 640 => actually 1280
    SHIZSize offset; // offset if letter/pillarboxing is enabled
} SHIZViewport;

typedef struct {
    GLuint program;
    GLuint vbo;
    GLuint vao;
} SHIZRenderData;

typedef struct {
    SHIZVector3 position;
    SHIZColor color;
} SHIZVertexPositionColor;

typedef struct {
    SHIZVector3 position;
    SHIZColor color;
    SHIZVector2 texture_coord;
    SHIZVector2 texture_coord_min;
    SHIZVector2 texture_coord_max;
} SHIZVertexPositionColorTexture;

#endif // internal_h
