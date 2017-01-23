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

#include "internal.h"

SHIZSpriteFontMeasurement shiz_sprite_measure_text(SHIZSpriteFont const font,
                                                   const char * const text,
                                                   SHIZSize const bounds,
                                                   SHIZSpriteFontAttributes const attributes);

SHIZSize shiz_sprite_draw_text(SHIZSpriteFont const font,
                               const char * const text,
                               SHIZVector2 const origin,
                               SHIZSpriteFontAlignment const alignment,
                               SHIZSize const bounds,
                               SHIZColor const tint,
                               SHIZSpriteFontAttributes const attributes,
                               SHIZColor * const highlight_colors,
                               uint const highlight_color_count);

#endif // spritefont_h
