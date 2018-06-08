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

#include "internal.h" // SHIZVector3, mat4x4
#include "viewport.h" // SHIZViewport

void z_transform__project_ortho(mat4x4 world, mat4x4 model, SHIZViewport viewport);
void z_transform__translate_rotate_scale(mat4x4 model, SHIZVector3 translation, float angle, float scale);
