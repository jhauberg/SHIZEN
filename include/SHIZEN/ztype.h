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

#ifndef ztype_h
#define ztype_h

#include <stdbool.h>

#include "zlayer.h"

typedef struct SHIZVector2 {
    float x, y;
} SHIZVector2;

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

typedef enum SHIZDrawMode {
    SHIZDrawModeFill,
    SHIZDrawModeOutline
} SHIZDrawMode;

/**
 * @brief Represents a 2-dimensional frame of an image resource.
 *
 * The source frame can either be a subset of, or span the entire image.
 */
typedef struct SHIZSprite {
    /* The frame that specifies which part of the image to draw */
    SHIZRect source;
    /** The image resource */
    uint8_t resource_id;
} SHIZSprite;

typedef enum SHIZSpriteFlipMode {
    SHIZSpriteFlipModeNone = 0,
    SHIZSpriteFlipModeVertical = 1,
    SHIZSpriteFlipModeHorizontal = 2
} SHIZSpriteFlipMode;

typedef struct SHIZSpriteSheet {
    SHIZSprite resource;
    SHIZSize sprite_size;
    SHIZSize sprite_padding;
    uint16_t columns;
    uint16_t rows;
} SHIZSpriteSheet;

typedef struct SHIZSpriteSize {
    SHIZSize target;
    SHIZVector2 scale;
} SHIZSpriteSize;

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
    uint32_t const * codepage;
    uint16_t columns;
    uint16_t rows;
} SHIZSpriteFontTable;

/**
 * @brief Represents a set of attributes that specify how text should be drawn.
 */
typedef struct SHIZSpriteFontAttributes {
    /** A scale defining the final size of the text */
    SHIZVector2 scale;
    /** A scale defining how "tight" characters are drawn */
    float character_spread;
    /** A value that adds padding to each character */
    float character_padding;
    /** A value that adds padding to each line */
    float line_padding;
    /** The word-wrapping mode */
    SHIZSpriteFontWrapMode wrap;
    /** A pointer to an array of tint colors */
    SHIZColor const * colors;
    /** The amount of colors these attributes point to */
    uint8_t colors_count;
} SHIZSpriteFontAttributes;

/**
 * @brief Represents a set of sprite characters aligned to an ASCII table.
 */
typedef struct SHIZSpriteFont {
    /** The ASCII table that aligns with the font sprite */
    SHIZSpriteFontTable table;
    /** A sprite that defines the font resource */
    SHIZSprite sprite;
    /** The size of each sprite character */
    SHIZSize character;
    /** Determines whether the font resource includes a sprite for the
      * whitespace character */
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

extern SHIZColor const SHIZColorWhite;
extern SHIZColor const SHIZColorBlack;
extern SHIZColor const SHIZColorRed;
extern SHIZColor const SHIZColorGreen;
extern SHIZColor const SHIZColorBlue;
extern SHIZColor const SHIZColorYellow;

extern SHIZSize const SHIZSizeZero;
extern SHIZRect const SHIZRectEmpty;

/**
 * @brief An empty sprite. This sprite cannot be drawn.
 */
extern SHIZSprite const SHIZSpriteEmpty;
extern SHIZSpriteSheet const SHIZSpriteSheetEmpty;
extern SHIZSpriteFont const SHIZSpriteFontEmpty;
extern SHIZSpriteFontTable const SHIZSpriteFontTableEmpty;

/**
 * @brief Size a sprite to its intrinsic (or natural) size.
 */
extern SHIZSpriteSize const SHIZSpriteSizeIntrinsic;

extern SHIZVector2 const SHIZAnchorCenter;
extern SHIZVector2 const SHIZAnchorTop;
extern SHIZVector2 const SHIZAnchorTopLeft;
extern SHIZVector2 const SHIZAnchorLeft;
extern SHIZVector2 const SHIZAnchorBottomLeft;
extern SHIZVector2 const SHIZAnchorBottom;
extern SHIZVector2 const SHIZAnchorTopRight;
extern SHIZVector2 const SHIZAnchorRight;
extern SHIZVector2 const SHIZAnchorBottomRight;

/**
 * @brief Do not apply scaling (scale = 1).
 */
#define SHIZSpriteNoScale 1.0f
/**
 * @brief Do not apply rotation.
 */
#define SHIZSpriteNoAngle 0.0f
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

#define SHIZSpriteFontAlignmentDefault (SHIZSpriteFontAlignmentTop|SHIZSpriteFontAlignmentLeft)
/**
 * @brief Do not constrain text to bounds.
 */
#define SHIZSpriteFontSizeToFit SHIZSizeMake(SHIZSpriteSizeIntrinsic.target.width, \
                                             SHIZSpriteSizeIntrinsic.target.height)
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
#define SHIZSpriteFontSpreadNormal 1.0f
/**
 * @brief Apply tight font spread; characters will be spaced with smaller gaps.
 */
#define SHIZSpriteFontSpreadTight 0.9f
/**
 * @brief Apply loose font spread; characters will be spaced with larger gaps.
 */
#define SHIZSpriteFontSpreadLoose 1.1f
/**
 * @brief Do not apply.
 */
#define SHIZSpriteFontNoPadding 0.0f

static inline
SHIZVector2 const
SHIZVector2Make(float const x, float const y)
{
    SHIZVector2 const vector = {
        x, y
    };

    return vector;
}

static inline
SHIZSize const
SHIZSizeMake(float const width, float const height)
{
    SHIZSize const size = {
        width, height
    };

    return size;
}

static inline
SHIZRect const
SHIZRectMake(SHIZVector2 const origin, SHIZSize const size)
{
    SHIZRect const rect = {
        origin, size
    };

    return rect;
}

static inline
SHIZRect
SHIZRectFromPoints(SHIZVector2 const points[], uint16_t const count)
{
    if (count == 0) {
        return SHIZRectEmpty;
    }
    
    SHIZVector2 min = points[0];
    SHIZVector2 max = points[0];
    
    for (uint8_t i = 1; i < count; i++) {
        SHIZVector2 const point = points[i];
        
        if (point.x < min.x) {
            min.x = point.x;
        }
        
        if (point.y < min.y) {
            min.y = point.y;
        }
        
        if (point.x > max.x) {
            max.x = point.x;
        }
        
        if (point.y > max.y) {
            max.y = point.y;
        }
    }
    
    float const width = max.x - min.x;
    float const height = max.y - min.y;
    
    SHIZSize const size = SHIZSizeMake(width, height);
    
    return SHIZRectMake(min, size);
}

static inline
SHIZVector2
SHIZVector2CenterFromPoints(SHIZVector2 const points[],
                            uint16_t const count)
{
    SHIZVector2 sum = SHIZVector2Zero;
    
    for (uint8_t i = 0; i < count; i++) {
        SHIZVector2 const point = points[i];
        
        sum.x += point.x;
        sum.y += point.y;
    }
    
    return SHIZVector2Make(sum.x / count,
                           sum.y / count);
}

static inline
SHIZRect const
SHIZRectMakeEx(float const x, float const y,
               float const width, float const height)
{
    return SHIZRectMake(SHIZVector2Make(x, y),
                        SHIZSizeMake(width, height));
}

static inline
SHIZColor const
SHIZColorMake(float const r,
              float const g,
              float const b,
              float const alpha)
{
    SHIZColor const color = {
        r, g, b, alpha
    };

    return color;
}

static inline
SHIZColor const
SHIZColorFromHex(int32_t const value)
{
    SHIZColor const color = SHIZColorMake(((value >> 16) & 0xFF) / 255.0f,
                                          ((value >> 8) & 0xFF) / 255.0f,
                                          ((value >> 0) & 0xFF) / 255.0f, 1);
    
    return color;
}

static inline
SHIZColor const
SHIZColorWithAlpa(SHIZColor const color, float const alpha)
{
    SHIZColor result_color = color;

    result_color.alpha = alpha;

    return result_color;
}

static inline
SHIZColor const
SHIZSpriteTintDefaultWithAlpa(float const alpha)
{
    return SHIZColorWithAlpa(SHIZSpriteNoTint, alpha);
}

static inline
SHIZSpriteSize const
SHIZSpriteSized(SHIZSize const size, SHIZVector2 const scale)
{
    SHIZSpriteSize sprite_size;

    sprite_size.target = size;
    sprite_size.scale = scale;

    return sprite_size;
}

static inline
SHIZSpriteSize const
SHIZSpriteSizedIntrinsicallyWithScale(SHIZVector2 const scale)
{
    SHIZSpriteSize size = SHIZSpriteSizeIntrinsic;

    size.scale = scale;

    return size;
}

static inline
SHIZSpriteFontAttributes const
SHIZSpriteFontAttributesWithScaleAndWrap(float const scale,
                                         SHIZSpriteFontWrapMode const wrap)
{
    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault;

    attrs.scale = SHIZVector2Make(scale, scale);
    attrs.wrap = wrap;

    return attrs;
}

static inline
SHIZSpriteFontAttributes const
SHIZSpriteFontAttributesWithScale(float const scale)
{
    SHIZSpriteFontWrapMode const wrap = SHIZSpriteFontAttributesDefault.wrap;
    
    return SHIZSpriteFontAttributesWithScaleAndWrap(scale, wrap);
}

static inline
SHIZSpriteFontAttributes const
SHIZSpriteFontAttributesWithWrap(SHIZSpriteFontWrapMode const wrap)
{
    return SHIZSpriteFontAttributesWithScaleAndWrap(SHIZSpriteNoScale, wrap);
}

static inline
SHIZVector2 const
SHIZAnchorInverse(SHIZVector2 const anchor)
{
    return SHIZVector2Make(anchor.x * -1,
                           anchor.y * -1);
}

#endif // ztype_h
