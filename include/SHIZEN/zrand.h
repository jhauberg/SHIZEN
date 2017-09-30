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

#ifndef zrand_h
#define zrand_h

#include "zint.h"
#include "ztype.h"

void
z_random_seed(u32 seed);

void
z_random_seed_now(void);

u32
z_random_get_seed(void);

i32
z_random_int(void);

i32
z_random_int_range(i32 const min,
                   i32 const max);

f32
z_random(void);

f32
z_random_range(f32 const min,
               f32 const max);

f32
z_random_angle(void);

SHIZColor
z_random_color(void);

#endif /* zrand_h */
