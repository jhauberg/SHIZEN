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

#include "internal.h"
#include "viewport.h"

void shiz_transform_project_ortho(mat4x4 world,
                                  mat4x4 model,
                                  SHIZViewport const viewport);

void shiz_transform_translate_rotate_scale(mat4x4 model,
                                           SHIZVector3 const translation,
                                           float const angle,
                                           float const scale);

#endif // transform_h
