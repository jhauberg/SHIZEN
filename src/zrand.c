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
#include <SHIZEN/zmath.h>
#include <SHIZEN/ztype.h> //

#include <stdlib.h> // NULL
#include <math.h> // ldexpf, M_PI
#include <time.h> // time()

#ifdef SHIZ_DEBUG
 #include "io.h"
#endif

#include <pcg/pcg_basic.h>

static uint32_t current_seed;

void
z_rand_seed(uint32_t seed)
{
    pcg32_srandom(seed, 0xda3e39cb94b95bdbULL);
    
    current_seed = seed;
    
#ifdef SHIZ_DEBUG
    z_io__debug("Seeding: %d", current_seed);
#endif
}

void
z_rand_seed_now()
{
    z_rand_seed((uint32_t)time(NULL));
}

uint32_t
z_rand_get_seed()
{
    return current_seed;
}

uint32_t
z_rand()
{
    return pcg32_random();
}

int32_t
z_rand_range(int32_t const min, int32_t const max)
{
    uint32_t const range = (uint32_t)abs(max - min);
    uint32_t const r = pcg32_boundedrand(range + 1); // both min and max inclusive
    
    int32_t const result = min + (int32_t)r;
    
    return result;
}

float
z_randf()
{
    return ldexpf(z_rand(), -32);
}

float
z_randf_range(float const min, float const max)
{
    return z_lerp(min, max, z_randf());
}

float
z_randf_angle()
{
    return z_randf_range((float)-M_PI, (float)M_PI);
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
z_rand_chance(float const percentage)
{
    return z_randf() < percentage;
}
