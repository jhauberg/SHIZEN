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

#include "internal.h"

/**
 * The amount of sprites that can be batched until a draw call is issued.
 */
#define SHIZSpriteInternalMax 2048

SHIZRect const shiz_sprite_get_anchored_rect(SHIZSize size,
                                             SHIZVector2 anchor);

void shiz_sprite_reset(void);
void shiz_sprite_flush(void);

SHIZSize const shiz_sprite_draw(SHIZSprite sprite,
                                SHIZVector2 origin,
                                SHIZSpriteSize size,
                                SHIZVector2 anchor,
                                float angle,
                                SHIZColor tint,
                                bool repeat,
                                bool opaque,
                                SHIZLayer layer);

#ifdef SHIZ_DEBUG
unsigned int shiz_debug_get_sprite_count(void);
#endif

#endif // sprite_h
