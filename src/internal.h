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

typedef struct SHIZGraphicsContext SHIZGraphicsContext;
typedef struct SHIZTimeLine SHIZTimeLine;
typedef struct SHIZViewport SHIZViewport;
typedef struct SHIZVertexPositionColor SHIZVertexPositionColor;
typedef struct SHIZVertexPositionColorTexture SHIZVertexPositionColorTexture;
typedef struct SHIZSpriteFontLine SHIZSpriteFontLine;
typedef struct SHIZSpriteFontMeasurement SHIZSpriteFontMeasurement;
typedef struct SHIZRenderData SHIZRenderData;

typedef struct SHIZSpriteInternal SHIZSpriteInternal;
typedef struct SHIZSpriteInternalKey SHIZSpriteInternalKey;

typedef enum SHIZViewportMode SHIZViewportMode;

extern SHIZGraphicsContext shiz_context;
#ifdef SHIZ_DEBUG
extern SHIZSpriteFont shiz_debug_font;
#endif

struct SHIZGraphicsContext {
    /** Determines whether the context has been initialized */
    bool is_initialized;
    /** Determines whether the context has focus */
    bool is_focused;
#ifdef SHIZ_DEBUG
    bool is_debug_enabled;
#endif
    /** Determines whether a shutdown should be initiated */
    bool should_finish;
    /** The preferred screen size; the display may be boxed if necessary */
    SHIZSize preferred_screen_size;
    /** A reference to the current window */
    GLFWwindow *window;
};

struct SHIZTimeLine {
    double time;
    double time_step;
    float scale;
};

enum SHIZViewportMode {
    SHIZViewportModeNormal,
    SHIZViewportModeLetterbox,
    SHIZViewportModePillarbox
};

struct SHIZViewport {
    SHIZSize framebuffer;
    SHIZSize screen;
    float scale; // framebuffer pixel scale; i.e. retina @2x framebuffer at 640 => actually 1280
    SHIZSize offset; // offset if letter/pillarboxing is enabled
    bool is_fullscreen;
};

struct SHIZRenderData {
    GLuint program;
    GLuint vbo;
    GLuint vao;
};

struct SHIZVertexPositionColor {
    SHIZVector3 position;
    SHIZColor color;
};

struct SHIZVertexPositionColorTexture {
    SHIZVector3 position;
    SHIZColor color;
    SHIZVector2 texture_coord;
    SHIZVector2 texture_coord_min;
    SHIZVector2 texture_coord_max;
};

// essentially an unsigned long for easy sorting
struct SHIZSpriteInternalKey {
    unsigned int layer_depth: 16;
    unsigned int layer: 7; // max 128
    unsigned int texture_id: 8; // max 256
    unsigned int is_opaque: 1;
};

#define SHIZSpriteInternalMax 2048

struct SHIZSpriteInternal {
    unsigned long key;
    SHIZVertexPositionColorTexture vertices[6];
    SHIZVector3 origin;
    float angle;
};

struct SHIZSpriteFontLine {
    SHIZSize size;
    uint ignored_character_count;
};

#define SHIZSpriteFontMaxLines 16

struct SHIZSpriteFontMeasurement {
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
    uint ignored_characters_count;
};

static const SHIZViewport SHIZViewportDefault = {
    .framebuffer = { .width = 0, .height = 0 },
    .screen = { .width = 0, .height = 0 },
    .scale = 1,
    .offset = { .width = 0, .height = 0 },
    .is_fullscreen = false
};

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
