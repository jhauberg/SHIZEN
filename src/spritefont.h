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

#ifndef spritefont_h
#define spritefont_h

#include <SHIZEN/ztype.h>

#define SHIZSpriteFontMaxLines 16

typedef struct SHIZSpriteFontLine {
    /** The measured size of the line of text */
    SHIZSize size;
    /** The number of characters in the line */
    unsigned int character_count;
} SHIZSpriteFontLine;

typedef struct SHIZSpriteFontMeasurement {
    /** The measured size of the entire text as a whole */
    SHIZSize size;
    /** The size of a character sprite as it should appear when drawn (may be scaled) */
    SHIZSize character_size;
    /** The size of a character sprite after applying any size-altering attributes (may be sized with
     offsets/padding, so these values are not suitable for drawing; use `character_size` instead) */
    SHIZSize character_size_perceived;
    /** A buffer holding the measured size of each line */
    SHIZSpriteFontLine lines[SHIZSpriteFontMaxLines];
    /** The index of the last character that can fit within specified bounds, if any; -1 otherwise */
    int max_characters;
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
} SHIZSpriteFontMeasurement;

SHIZSpriteFontMeasurement const z_spritefont__measure_text(SHIZSpriteFont font,
                                                           char const * text,
                                                           SHIZSize bounds,
                                                           SHIZSpriteFontAttributes attrs);

SHIZSize const z_spritefont__draw_text(SHIZSpriteFont font,
                                       char const * text,
                                       SHIZVector2 origin,
                                       SHIZSpriteFontAlignment alignment,
                                       SHIZSize bounds,
                                       SHIZSpriteFontAttributes attrs,
                                       SHIZColor tint,
                                       SHIZLayer layer);

#endif // spritefont_h
