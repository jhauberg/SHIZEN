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

#include "internal_type.h"

bool z_gfx__init_spritebatch(void);
bool z_gfx__kill_spritebatch(void);

void z_gfx__spritebatch_reset(void);
bool z_gfx__spritebatch_flush(void);

void z_gfx__add_sprite(SHIZVertexPositionColorTexture const * restrict vertices,
                       SHIZVector3 origin,
                       float angle,
                       GLuint texture_id);

#endif // spritebatch_h
