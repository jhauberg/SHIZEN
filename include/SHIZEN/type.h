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

typedef struct {
    float x, y;
} SHIZVector2;

typedef struct {
    float x, y, z;
} SHIZVector3;

static const SHIZVector2 SHIZVector2Zero = { 0, 0 };
static const SHIZVector2 SHIZVector2One = { 1, 1 };
static const SHIZVector3 SHIZVector3Zero = { 0, 0, 0 };
static const SHIZVector3 SHIZVector3One = { 1, 1, 1 };

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

typedef struct {
    float width;
    float height;
} SHIZSize;

static const SHIZSize SHIZSizeEmpty = { 0, 0 };

static inline SHIZSize SHIZSizeMake(float const width, float const height) {
    SHIZSize size = {
        width, height
    };

    return size;
}

typedef struct {
    SHIZVector2 origin;
    SHIZSize size;
} SHIZRect;

static const SHIZRect SHIZRectEmpty = {
    { 0, 0 }, { 0, 0 }
};

static inline SHIZRect SHIZRectMake(SHIZVector2 origin, SHIZSize size) {
    SHIZRect rect = {
        origin, size
    };

    return rect;
}

typedef struct {
    float r, g, b;
    float alpha;
} SHIZColor;

static const SHIZColor SHIZColorWhite = { 1, 1, 1, 1 };
static const SHIZColor SHIZColorBlack = { 0, 0, 0, 1 };
static const SHIZColor SHIZColorRed = { 1, 0, 0, 1 };
static const SHIZColor SHIZColorGreen = { 0, 1, 0, 1 };
static const SHIZColor SHIZColorBlue = { 0, 0, 1, 1 };

static inline SHIZColor SHIZColorMake(float const r, float const g, float const b, float const alpha) {
    SHIZColor color = {
        r, g, b, alpha
    };

    return color;
}

typedef struct {
    uint resource_id;
    SHIZRect source;
} SHIZSprite;

static const SHIZSprite SHIZSpriteEmpty = {
    0, { { 0, 0 }, { 0, 0 } }
};

static const SHIZVector2 SHIZSpriteAnchorCenter = { 0.0f, 0.0f };
static const SHIZVector2 SHIZSpriteAnchorTop = { 0.0f, 1.0f };
static const SHIZVector2 SHIZSpriteAnchorTopLeft = { -1.0f, 1.0f };
static const SHIZVector2 SHIZSpriteAnchorLeft = { -1.0f, 0.0f };
static const SHIZVector2 SHIZSpriteAnchorBottomLeft = { -1.0f, -1.0f };
static const SHIZVector2 SHIZSpriteAnchorBottom = { 0.0f, -1.0f };
static const SHIZVector2 SHIZSpriteAnchorTopRight = { 1.0f, 1.0f };
static const SHIZVector2 SHIZSpriteAnchorRight = { 1.0f, 0.0f };
static const SHIZVector2 SHIZSpriteAnchorBottomRight = { 1.0f, -1.0f };

static const SHIZSize SHIZSpriteSizeIntrinsic = { -1, -1 };

#define SHIZSpriteNoTint SHIZColorWhite

#define SHIZSpriteRepeat true
#define SHIZSpriteNoRepeat false

static const float SHIZSpriteNoAngle = 0;
static const uint SHIZSpriteLayerDefault = 0;

static inline const SHIZColor SHIZSpriteTintDefaultWithAlpa(float const alpha) {
    SHIZColor default_tint = SHIZSpriteNoTint;
    
    default_tint.alpha = alpha;
    
    return default_tint;
}

typedef struct {
    uint columns;
    uint rows;
    uint offset;
} SHIZASCIITable;

typedef struct {
    SHIZSprite sprite;
    SHIZSize character;
    SHIZASCIITable table;
    bool includes_whitespace;
} SHIZSpriteFont;

typedef enum {
    SHIZSpriteFontAlignmentTop = 1,
    SHIZSpriteFontAlignmentLeft = 2,
    SHIZSpriteFontAlignmentRight = 4,
    SHIZSpriteFontAlignmentCenter = 8,
    SHIZSpriteFontAlignmentMiddle = 16,
    SHIZSpriteFontAlignmentBottom = 32
} SHIZSpriteFontAlignment;

typedef enum {
    SHIZSpriteFontWrapModeCharacter,
    SHIZSpriteFontWrapModeWord
} SHIZSpriteFontWrapMode;

typedef struct {
    SHIZSpriteFontWrapMode wrap;
    SHIZVector2 scale; /* a scale defining the final size of the text */
    float character_spread; /* a scale defining how "tight" characters are drawn */
    float character_padding; /* a value that adds padding to each character */
    float line_padding; /* a value that adds padding to each line */
} SHIZSpriteFontAttributes;

#define SHIZSpriteFontSizeToFit SHIZSpriteSizeIntrinsic
#define SHIZSpriteFontSizeToFitHorizontally SHIZSpriteFontSizeToFit.width
#define SHIZSpriteFontSizeToFitVertically SHIZSpriteFontSizeToFit.height
#define SHIZSpriteFontSpreadNormal 1.0
#define SHIZSpriteFontSpreadTight 0.9
#define SHIZSpriteFontSpreadLoose 1.1

#define SHIZSpriteFontNoPadding 0

static const SHIZSpriteFontAttributes SHIZSpriteFontAttributesDefault = {
    SHIZSpriteFontWrapModeWord, { 1, 1 }, SHIZSpriteFontSpreadNormal, SHIZSpriteFontNoPadding, SHIZSpriteFontNoPadding
};

#endif // type_h
