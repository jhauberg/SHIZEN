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

#ifndef spritebatch_h
#define spritebatch_h

#include "internal.h"

bool shiz_gfx_init_spritebatch(void);
bool shiz_gfx_kill_spritebatch(void);

void shiz_gfx_spritebatch_reset(void);
bool shiz_gfx_spritebatch_flush(void);

void shiz_gfx_add_sprite(SHIZVertexPositionColorTexture const * restrict vertices,
                         SHIZVector3 origin,
                         float angle,
                         GLuint texture_id);

#endif // spritebatch_h
