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
#include <stdint.h>

typedef struct SHIZVector2 SHIZVector2;
typedef struct SHIZVector3 SHIZVector3;
typedef struct SHIZColor SHIZColor;
typedef struct SHIZSize SHIZSize;
typedef struct SHIZRect SHIZRect;
typedef struct SHIZSprite SHIZSprite;
typedef struct SHIZSpriteFont SHIZSpriteFont;
typedef struct SHIZSpriteFontTable SHIZSpriteFontTable;
typedef struct SHIZSpriteFontAttributes SHIZSpriteFontAttributes;

typedef enum SHIZSpriteFontAlignment SHIZSpriteFontAlignment;
typedef enum SHIZSpriteFontWrapMode SHIZSpriteFontWrapMode;

struct SHIZVector2 {
    float x, y;
};

struct SHIZVector3 {
    float x, y, z;
};

struct SHIZSize {
    float width;
    float height;
};

struct SHIZRect {
    SHIZVector2 origin;
    SHIZSize size;
};

struct SHIZColor {
    float r, g, b;
    float alpha;
};

/**
 * Represents a 2-dimensional frame of an image resource.
 * The source frame can either be a subset of, or span the entire image.
 */
struct SHIZSprite {
    /** The image resource */
    uint resource_id;
    /* The frame that specifies which part of the image to draw */
    SHIZRect source;
};

enum SHIZSpriteFontAlignment {
    SHIZSpriteFontAlignmentTop = 1,
    SHIZSpriteFontAlignmentLeft = 2,
    SHIZSpriteFontAlignmentRight = 4,
    SHIZSpriteFontAlignmentCenter = 8,
    SHIZSpriteFontAlignmentMiddle = 16,
    SHIZSpriteFontAlignmentBottom = 32
};

enum SHIZSpriteFontWrapMode {
    SHIZSpriteFontWrapModeCharacter,
    SHIZSpriteFontWrapModeWord
};

struct SHIZSpriteFontTable {
    uint columns;
    uint rows;
    /** An offset to align the table with the standard ASCII table */
    uint offset;
};

/**
 * Represents a set of attributes that specify how text should be drawn.
 */
struct SHIZSpriteFontAttributes {
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
};

/**
 Represents a set of sprite characters aligned to an ASCII table.
 */
struct SHIZSpriteFont {
    /** A sprite that defines the font resource */
    SHIZSprite sprite;
    /** The size of each sprite character */
    SHIZSize character;
    /** The ASCII table that aligns with the font sprite */
    SHIZSpriteFontTable table;
    /** Determines whether the font resource includes a sprite for the whitespace character */
    bool includes_whitespace;
};

/**
 * @brief Default font attributes.
 * 
 * Default font attributes apply scaling at 1:1 and enables word-wrapping.
 */
extern const SHIZSpriteFontAttributes SHIZSpriteFontAttributesDefault;

extern const SHIZVector2 SHIZVector2Zero;
extern const SHIZVector2 SHIZVector2One;
extern const SHIZVector3 SHIZVector3Zero;
extern const SHIZVector3 SHIZVector3One;

extern const SHIZColor SHIZColorWhite;
extern const SHIZColor SHIZColorBlack;
extern const SHIZColor SHIZColorRed;
extern const SHIZColor SHIZColorGreen;
extern const SHIZColor SHIZColorBlue;

extern const SHIZSize SHIZSizeEmpty;
extern const SHIZRect SHIZRectEmpty;

/**
 * @brief An empty sprite. This sprite cannot be drawn.
 */
extern const SHIZSprite SHIZSpriteEmpty;

/**
 * @brief Size a sprite to its intrinsic (or natural) size.
 */
extern const SHIZSize SHIZSpriteSizeIntrinsic;

/**
 * @brief Do not apply rotation.
 */
extern const float SHIZSpriteNoAngle;
/**
 * @brief Default sprite layer.
 */
extern const uint SHIZSpriteLayerDefault;

extern const SHIZVector2 SHIZSpriteAnchorCenter;
extern const SHIZVector2 SHIZSpriteAnchorTop;
extern const SHIZVector2 SHIZSpriteAnchorTopLeft;
extern const SHIZVector2 SHIZSpriteAnchorLeft;
extern const SHIZVector2 SHIZSpriteAnchorBottomLeft;
extern const SHIZVector2 SHIZSpriteAnchorBottom;
extern const SHIZVector2 SHIZSpriteAnchorTopRight;
extern const SHIZVector2 SHIZSpriteAnchorRight;
extern const SHIZVector2 SHIZSpriteAnchorBottomRight;

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

static inline SHIZVector2 SHIZVector2Make(float const x, float const y) {
    SHIZVector2 vector = {
        x, y
    };

    return vector;
}

static inline SHIZVector3 SHIZVector3Make(float const x, float const y, float const z) {
    SHIZVector3 vector = {
        x, y, z
    };

    return vector;
}

static inline SHIZSize SHIZSizeMake(float const width, float const height) {
    SHIZSize size = {
        width, height
    };

    return size;
}

static inline SHIZRect SHIZRectMake(SHIZVector2 origin, SHIZSize size) {
    SHIZRect rect = {
        origin, size
    };

    return rect;
}

static inline SHIZColor SHIZColorMake(float const r, float const g, float const b, float const alpha) {
    SHIZColor color = {
        r, g, b, alpha
    };

    return color;
}

static inline SHIZColor SHIZColorFromHex(int const value) {
    SHIZColor color = SHIZColorMake(((value >> 16) & 0xFF) / 255.0f,
                                    ((value >> 8) & 0xFF) / 255.0f,
                                    ((value >> 0) & 0xFF) / 255.0f, 1);
    
    return color;
}

static inline const SHIZColor SHIZSpriteTintDefaultWithAlpa(float const alpha) {
    SHIZColor default_tint = SHIZSpriteNoTint;

    default_tint.alpha = alpha;

    return default_tint;
}

#endif // type_h
