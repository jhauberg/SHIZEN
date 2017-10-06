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

#include <stdlib.h> // NULL
#include <math.h> // ldexpf, M_PI
#include <time.h> // time()

#ifdef SHIZ_DEBUG
 #include <stdio.h>
#endif

#include <pcg_basic.h>

static u32 current_seed;

void
z_rand_seed(u32 seed)
{
    pcg32_srandom(seed, 0xda3e39cb94b95bdbULL);
    
    current_seed = seed;
    
#ifdef SHIZ_DEBUG
    printf("Seeding: %d\n", current_seed);
#endif
}

void
z_rand_seed_now()
{
    z_rand_seed((u32)time(NULL));
}

u32
z_rand_get_seed()
{
    return current_seed;
}

u32
z_rand()
{
    return pcg32_random();
}

i32
z_rand_range(i32 const min, i32 const max)
{
    u32 const range = (u32)abs(max - min);
    u32 const r = pcg32_boundedrand(range + 1); // both min and max inclusive
    
    i32 const result = min + (i32)r;
    
    return result;
}

f32
z_randf()
{
    return ldexpf(z_rand(), -32);
}

f32
z_randf_range(f32 const min, f32 const max)
{
    return z_lerp(min, max, z_randf());
}

f32
z_randf_angle()
{
    return z_randf_range((f32)-M_PI, (f32)M_PI);
}

SHIZColor
z_rand_color()
{
    return SHIZColorMake(z_randf(), z_randf(), z_randf(), 1);
}

bool
z_rand_choice()
{
    return z_rand_range(0, 1) > 0;
}

bool
z_rand_chance(f32 const percentage)
{
    return z_randf() < percentage;
}
