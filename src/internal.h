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

static const SHIZViewport SHIZViewportDefault = { { 0, 0 }, { 0, 0 }, 1, { 0, 0 } };

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

static uint const SHIZSpriteFontMaxLines = 16;

typedef struct {
    /** The measured size of the entire text as a whole */
    SHIZSize size;
    /** A buffer holding the measured size of each line */
    SHIZSize line_size[SHIZSpriteFontMaxLines];
    /** The size of a character sprite as it should appear when drawn (may be scaled) */
    SHIZSize character_size;
    /** The size of a character sprite after applying any size-altering attributes (may be sized with
        offsets/padding, so these values are not suitable for drawing; use `character_size` instead) */
    SHIZSize character_size_perceived;
    /** The number of lines */
    uint line_count;
    /** The max number of characters per line before a linebreak is forced */
    uint max_characters_per_line;
    /** The max number of lines that can fit within specified bounds, if any */
    uint max_lines_in_bounds;
    /** Determines whether to keep text within horizontal bounds */
    bool constrain_horizontally;
    /** Determines whether to keep text within vertical bounds, forcing linebreaks if possible; truncates otherwise */
    bool constrain_vertically;
    /** The index of the last character that can fit within specified bounds, if any; -1 otherwise */
    int constrain_index;
} SHIZSpriteFontMeasurement;

static inline uint const _shiz_get_char_size(char const character) {
    int bits = 7;
    uint size = 0;
    
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

#endif // internal_h
