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

#include <SHIZEN/zrand.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef SHIZ_DEBUG
 #include <stdio.h>
#endif

static u32 current_seed;

void
z_random_seed(u32 seed)
{
    srand(seed);
    
    current_seed = seed;
    
#ifdef SHIZ_DEBUG
    printf("Seeding: %d\n", current_seed);
#endif
}

void
z_random_seed_now()
{
    z_random_seed((u32)time(NULL));
}

u32
z_random_get_seed()
{
    return current_seed;
}

i32
z_random_int()
{
    return rand();
}

i32
z_random_int_range(i32 const min, i32 const max)
{
    return min + (z_random_int() % ((max + 1) - min));
}

f32
z_random()
{
    return rand() / (RAND_MAX + 1.0f);
}

f32
z_random_range(f32 const min, f32 const max)
{
    return z_lerp(min, max, z_random());
}

f32
z_random_angle()
{
    return z_random_range((f32)-M_PI,
                          (f32)M_PI);
}

SHIZColor
z_random_color()
{
    return SHIZColorMake(z_random(),
                         z_random(),
                         z_random(),
                         1);
}
