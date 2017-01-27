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

#include "type.h"

#define SHIZEpsilon (1.0 / 1024)

#define SHIZSpriteInternalMax 2048
#define SHIZSpriteFontMaxLines 16

typedef struct SHIZGraphicsContext {
    /** Determines whether the context has been initialized */
    bool is_initialized;
    /** Determines whether the context has focus */
    bool is_focused;
    /** Determines whether a shutdown should be initiated */
    bool should_finish;
    /** The preferred screen size; the display may be boxed if necessary */
    SHIZSize preferred_screen_size;
    /** A reference to the current window */
    GLFWwindow *window;
} SHIZGraphicsContext;

extern SHIZGraphicsContext shiz_context;

typedef struct SHIZTimeLine {
    double time;
    double time_step;
    double scale;
} SHIZTimeLine;

typedef enum SHIZViewportMode {
    SHIZViewportModeNormal,
    SHIZViewportModeLetterbox,
    SHIZViewportModePillarbox
} SHIZViewportMode;

typedef struct SHIZViewport {
    SHIZSize framebuffer;
    SHIZSize screen;
    float scale; // framebuffer pixel scale; i.e. retina @2x framebuffer at 640 => actually 1280
    SHIZSize offset; // offset if letter/pillarboxing is enabled
    bool is_fullscreen;
} SHIZViewport;

typedef struct SHIZVertexPositionColor {
    SHIZVector3 position;
    SHIZColor color;
} SHIZVertexPositionColor;

typedef struct SHIZVertexPositionColorTexture {
    SHIZVector3 position;
    SHIZColor color;
    SHIZVector2 texture_coord;
    SHIZVector2 texture_coord_min;
    SHIZVector2 texture_coord_max;
} SHIZVertexPositionColorTexture;

typedef struct SHIZSpriteInternalKey {
    bool is_transparent: 1; // the least significant bit
    unsigned short texture_id: 7;
    SHIZLayer layer; // 24 bits
} SHIZSpriteInternalKey; // total 32 bits (unsigned long)

typedef struct SHIZSpriteInternal {
    unsigned long key; // packed SHIZSpriteInternalKey
    unsigned int order; // literal call order; used as a last resort to ensure stable sorting
    SHIZVertexPositionColorTexture vertices[6];
    SHIZVector3 origin;
    float angle;
} SHIZSpriteInternal;

typedef struct SHIZSpriteFontLine {
    /** The measured size of the line of text */
    SHIZSize size;
    /** The number of encountered special/ignored characters in the line of text;
     typically counts tint specifiers */
    unsigned int ignored_character_count;
} SHIZSpriteFontLine;

typedef struct SHIZSpriteFontMeasurement {
    /** The measured size of the entire text as a whole */
    SHIZSize size;
    /** A buffer holding the measured size of each line */
    SHIZSpriteFontLine lines[SHIZSpriteFontMaxLines];
    /** The size of a character sprite as it should appear when drawn (may be scaled) */
    SHIZSize character_size;
    /** The size of a character sprite after applying any size-altering attributes (may be sized with
     offsets/padding, so these values are not suitable for drawing; use `character_size` instead) */
    SHIZSize character_size_perceived;
    /** The number of lines */
    unsigned int line_count;
    /** The max number of characters per line before a linebreak is forced */
    unsigned int max_characters_per_line;
    /** The max number of lines that can fit within specified bounds, if any */
    unsigned int max_lines_in_bounds;
    /** Determines whether to keep text within horizontal bounds */
    bool constrain_horizontally;
    /** Determines whether to keep text within vertical bounds, 
     forcing linebreaks if possible; truncates otherwise */
    bool constrain_vertically;
    /** The index of the last character that can fit within specified bounds, if any; -1 otherwise */
    int constrain_index;
} SHIZSpriteFontMeasurement;

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

#ifdef SHIZ_DEBUG
typedef struct SHIZDebugEvent {
    SHIZVector3 origin;
    const char * name;
    unsigned int lane;
} SHIZDebugEvent;

#define SHIZDebugEventMax 64

#define SHIZDebugEventLaneDraws 0
#define SHIZDebugEventLaneResources 1

typedef struct SHIZDebugContext {
    bool is_enabled;
    bool is_tracking_enabled; // used to disable event/draw call tracking while drawing debug stuff
    bool draw_sprite_shape;
    bool draw_events;
    unsigned int sprite_count;
    unsigned int event_count;
    SHIZDebugEvent events[SHIZDebugEventMax];
} SHIZDebugContext;

extern SHIZDebugContext shiz_debug_context;
extern SHIZSpriteFont shiz_debug_font;
#endif

#endif // internal_h
