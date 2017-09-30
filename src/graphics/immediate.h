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

#ifndef immediate_h
#define immediate_h

#include "internal_type.h"

bool
z_gfx__init_immediate(void);

bool
z_gfx__kill_immediate(void);

void
z_gfx__render_immediate(GLenum mode,
                        SHIZVertexPositionColor const * restrict vertices,
                        u32 count,
                        SHIZVector3 origin,
                        f32 angle);

#endif /* immediate_h */
