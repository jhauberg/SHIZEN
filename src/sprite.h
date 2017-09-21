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

#ifndef sprite_h
#define sprite_h

#include <SHIZEN/ztype.h>

/**
 * The amount of sprites that can be sorted per frame before a batch is issued.
 */
#define SHIZSpriteMax 2048

SHIZRect const
z_sprite__anchor_rect(SHIZSize size,
                      SHIZVector2 anchor);

void
z_sprite__reset(void);

void
z_sprite__flush(void);

SHIZSize const
z_sprite__draw(SHIZSprite sprite,
               SHIZVector2 origin,
               SHIZSpriteSize size,
               bool repeat,
               SHIZVector2 anchor,
               f32 angle,
               SHIZColor tint,
               bool opaque,
               SHIZLayer layer);

#endif // sprite_h
