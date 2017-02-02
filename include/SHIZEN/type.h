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

#ifndef type_h
#define type_h

#include <stdbool.h>

typedef struct SHIZVector2 {
    float x, y;
} SHIZVector2;

typedef struct SHIZVector3 {
    float x, y, z;
} SHIZVector3;

typedef struct SHIZSize {
    float width;
    float height;
} SHIZSize;

typedef struct SHIZRect {
    SHIZVector2 origin;
    SHIZSize size;
} SHIZRect;

typedef struct SHIZColor {
    float r, g, b;
    float alpha;
} SHIZColor;

typedef struct SHIZLayer {
    unsigned short depth: 16;
    unsigned short layer: 8;
} SHIZLayer;

typedef enum SHIZDrawMode {
    SHIZDrawModeFill,
    SHIZDrawModeOutline
} SHIZDrawMode;

/**
 * Represents a 2-dimensional frame of an image resource.
 * The source frame can either be a subset of, or span the entire image.
 */
typedef struct SHIZSprite {
    /** The image resource */
    unsigned int resource_id;
    /* The frame that specifies which part of the image to draw */
    SHIZRect source;
} SHIZSprite;

typedef enum SHIZSpriteFontAlignment {
    SHIZSpriteFontAlignmentTop = 1,
    SHIZSpriteFontAlignmentLeft = 2,
    SHIZSpriteFontAlignmentRight = 4,
    SHIZSpriteFontAlignmentCenter = 8,
    SHIZSpriteFontAlignmentMiddle = 16,
    SHIZSpriteFontAlignmentBottom = 32
} SHIZSpriteFontAlignment;

typedef enum SHIZSpriteFontWrapMode {
    SHIZSpriteFontWrapModeCharacter,
    SHIZSpriteFontWrapModeWord
} SHIZSpriteFontWrapMode;

typedef struct SHIZSpriteFontTable {
    unsigned int columns;
    unsigned int rows;
    /** An offset to align the table with the standard ASCII table */
    unsigned int offset;
} SHIZSpriteFontTable;

/**
 * Represents a set of attributes that specify how text should be drawn.
 */
typedef struct SHIZSpriteFontAttributes {
    /** The word-wrapping mode */
    SHIZSpriteFontWrapMode wrap;
    /** A scale defining the final size of the text */
    SHIZVector2 scale;
    /** A scale defining how "tight" characters are drawn */
    float character_spread;
    /** A value that adds padding to each character */
    float character_padding;
    /** A value that adds padding to each line */
    float line_padding;
} SHIZSpriteFontAttributes;

/**
 Represents a set of sprite characters aligned to an ASCII table.
 */
typedef struct SHIZSpriteFont {
    /** A sprite that defines the font resource */
    SHIZSprite sprite;
    /** The size of each sprite character */
    SHIZSize character;
    /** The ASCII table that aligns with the font sprite */
    SHIZSpriteFontTable table;
    /** Determines whether the font resource includes a sprite for the whitespace character */
    bool includes_whitespace;
} SHIZSpriteFont;

/**
 * @brief Default font attributes.
 * 
 * Default font attributes apply scaling at 1:1 and enables word-wrapping.
 */
extern SHIZSpriteFontAttributes const SHIZSpriteFontAttributesDefault;

extern SHIZVector2 const SHIZVector2Zero;
extern SHIZVector2 const SHIZVector2One;
extern SHIZVector3 const SHIZVector3Zero;
extern SHIZVector3 const SHIZVector3One;

extern SHIZColor const SHIZColorWhite;
extern SHIZColor const SHIZColorBlack;
extern SHIZColor const SHIZColorRed;
extern SHIZColor const SHIZColorGreen;
extern SHIZColor const SHIZColorBlue;
extern SHIZColor const SHIZColorYellow;

extern SHIZSize const SHIZSizeEmpty;
extern SHIZRect const SHIZRectEmpty;

/**
 * @brief An empty sprite. This sprite cannot be drawn.
 */
extern SHIZSprite const SHIZSpriteEmpty;

/**
 * @brief Size a sprite to its intrinsic (or natural) size.
 */
extern SHIZSize const SHIZSpriteSizeIntrinsic;

extern SHIZLayer const SHIZLayerTop;
extern SHIZLayer const SHIZLayerBottom;

extern SHIZVector2 const SHIZAnchorCenter;
extern SHIZVector2 const SHIZAnchorTop;
extern SHIZVector2 const SHIZAnchorTopLeft;
extern SHIZVector2 const SHIZAnchorLeft;
extern SHIZVector2 const SHIZAnchorBottomLeft;
extern SHIZVector2 const SHIZAnchorBottom;
extern SHIZVector2 const SHIZAnchorTopRight;
extern SHIZVector2 const SHIZAnchorRight;
extern SHIZVector2 const SHIZAnchorBottomRight;

#define SHIZLayerMin 0
#define SHIZLayerMax 255 // UCHAR_MAX

#define SHIZLayerDepthMin 0
#define SHIZLayerDepthMax 65535 // USHRT_MAX

#define SHIZLayerDefault SHIZLayerBottom

/**
 * @brief Do not apply rotation.
 */
#define SHIZSpriteNoAngle 0
/**
 * @brief Do not apply a tint.
 */
#define SHIZSpriteNoTint SHIZColorWhite
/**
 * @brief Repeat the sprite.
 */
#define SHIZSpriteRepeat true
/**
 * @brief Do not repeat the sprite.
 */
#define SHIZSpriteNoRepeat false
/**
 * @brief The sprite contains transparent pixels.
 */
#define SHIZSpriteNotOpaque false
/**
 * @brief The sprite does not contain transparent pixels.
 */
#define SHIZSpriteIsOpaque true

/**
 * @brief Do not constrain text to bounds.
 */
#define SHIZSpriteFontSizeToFit SHIZSpriteSizeIntrinsic
/**
 * @brief Do not constrain text to horizontal bounds.
 */
#define SHIZSpriteFontSizeToFitHorizontally SHIZSpriteFontSizeToFit.width
/**
 * @brief Do not constrain text to vertical bounds.
 */
#define SHIZSpriteFontSizeToFitVertically SHIZSpriteFontSizeToFit.height
/**
 * @brief Apply normal font spread; characters will be spaced normally.
 */
#define SHIZSpriteFontSpreadNormal 1.0
/**
 * @brief Apply tight font spread; characters will be spaced with smaller gaps.
 */
#define SHIZSpriteFontSpreadTight 0.9
/**
 * @brief Apply loose font spread; characters will be spaced with larger gaps.
 */
#define SHIZSpriteFontSpreadLoose 1.1
/**
 * @brief Do not apply.
 */
#define SHIZSpriteFontNoPadding 0

// rule of thumb: small functions that do basic things (such as the make functions) and called frequently
// can be inlined at a benefit; think of it as the body of the function being put in place of the call
// so larger functions that do more than just initialize a struct shold not be inlined
static inline SHIZVector2 const
SHIZVector2Make(float const x, float const y) {
    SHIZVector2 const vector = {
        x, y
    };

    return vector;
}

static inline SHIZVector3 const
SHIZVector3Make(float const x, float const y, float const z) {
    SHIZVector3 const vector = {
        x, y, z
    };

    return vector;
}

static inline SHIZSize const
SHIZSizeMake(float const width, float const height) {
    SHIZSize const size = {
        width, height
    };

    return size;
}

static inline SHIZRect const
SHIZRectMake(SHIZVector2 origin, SHIZSize size) {
    SHIZRect const rect = {
        origin, size
    };

    return rect;
}

static inline SHIZRect const
SHIZRectMakeEx(float const x, float const y, float const width, float const height) {
    return SHIZRectMake(SHIZVector2Make(x, y),
                        SHIZSizeMake(width, height));
}

static inline SHIZColor const
SHIZColorMake(float const r,
              float const g,
              float const b,
              float const alpha) {
    SHIZColor const color = {
        r, g, b, alpha
    };

    return color;
}

static inline SHIZColor const
SHIZColorFromHex(int const value) {
    SHIZColor const color = SHIZColorMake(((value >> 16) & 0xFF) / 255.0f,
                                          ((value >> 8) & 0xFF) / 255.0f,
                                          ((value >> 0) & 0xFF) / 255.0f, 1);
    
    return color;
}

static inline SHIZColor const
SHIZColorWithAlpa(SHIZColor const color, float const alpha) {
    SHIZColor result_color = color;

    result_color.alpha = alpha;

    return result_color;
}

static inline SHIZColor const
SHIZSpriteTintDefaultWithAlpa(float const alpha) {
    return SHIZColorWithAlpa(SHIZSpriteNoTint, alpha);
}

static inline SHIZSpriteFontAttributes const
SHIZSpriteFontAttributesWithScaleAndWrap(float const scale,
                                         SHIZSpriteFontWrapMode const wrap) {
    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault;

    attrs.scale = SHIZVector2Make(scale, scale);
    attrs.wrap = wrap;

    return attrs;
}

static inline SHIZSpriteFontAttributes const
SHIZSpriteFontAttributesWithScale(float const scale) {
    return SHIZSpriteFontAttributesWithScaleAndWrap(scale, SHIZSpriteFontAttributesDefault.wrap);
}

static inline SHIZSpriteFontAttributes const
SHIZSpriteFontAttributesWithWrap(SHIZSpriteFontWrapMode const wrap) {
    return SHIZSpriteFontAttributesWithScaleAndWrap(SHIZSpriteFontAttributesDefault.scale.x, wrap);
}

static inline SHIZLayer const
SHIZLayeredWithDepth(unsigned char const layer,
                     unsigned short const depth) {
    SHIZLayer result;
    
    result.layer = layer;
    result.depth = depth;
    
    return result;
}

static inline SHIZLayer const
SHIZLayered(unsigned char const layer) {
    return SHIZLayeredWithDepth(layer, SHIZLayerDefault.depth);
}

static inline SHIZLayer const
SHIZLayeredBelow(SHIZLayer const layer) {
    SHIZLayer layer_below = layer;

    if (layer.depth > SHIZLayerDepthMin) {
        layer_below.depth = layer.depth - 1;
    } else if (layer.layer > SHIZLayerMin) {
        layer_below.layer = layer.layer - 1;
    }

    return layer_below;
}

static inline SHIZLayer const
SHIZLayeredAbove(SHIZLayer const layer) {
    SHIZLayer layer_above = layer;
    
    if (layer.depth < SHIZLayerDepthMax) {
        layer_above.depth = layer.depth + 1;
    } else if (layer.layer < SHIZLayerMax) {
        layer_above.layer = layer.layer + 1;
    }
    
    return layer_above;
}

#endif // type_h
