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

#ifndef transform_h
#define transform_h

#include "internal_type.h"
#include "viewport.h"

void
z_transform__project_ortho(mat4x4 world,
                           mat4x4 model,
                           SHIZViewport viewport);

void
z_transform__translate_rotate_scale(mat4x4 model,
                                    SHIZVector3 translation,
                                    f32 angle,
                                    f32 scale);

#endif // transform_h
