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

#include <stdbool.h>

void
z_rand_seed(u32 seed);

void
z_rand_seed_now(void);

u32
z_rand_get_seed(void);

u32
z_rand(void);

s32
z_rand_range(s32 const min,
             s32 const max);

f32
z_randf(void);

f32
z_randf_range(f32 const min,
              f32 const max);

f32
z_randf_angle(void);

SHIZColor
z_rand_color(void);

bool
z_rand_choice(void);

bool
z_rand_chance(f32 percentage);

#endif /* zrand_h */
