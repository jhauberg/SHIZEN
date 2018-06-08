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

#pragma once

#include <stdint.h> // uint32_t

#include "../internal.h" // SHIZVertexPositionColor, SHIZVector3, GLenum

bool z_gfx__init_immediate(void);
bool z_gfx__kill_immediate(void);

void z_gfx__render_immediate(GLenum mode, SHIZVertexPositionColor const * restrict vertices, uint32_t count, SHIZVector3 origin, float angle);
