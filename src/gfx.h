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

#ifndef gfx_h
#define gfx_h

#include "internal.h"

bool shiz_gfx_init(void);
bool shiz_gfx_kill(void);

void shiz_gfx_render(GLenum const mode, SHIZVertexPositionColor const *vertices, uint const count);

#endif // gfx_h
