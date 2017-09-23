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

#ifndef internal_h
#define internal_h

#define SHIZEN_VERSION_MAJOR 0
#define SHIZEN_VERSION_MINOR 19
#define SHIZEN_VERSION_PATCH 1

#define SHIZEN_VERSION_NAME "ALPHA"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <SHIZEN/zint.h>

#define SHIZEpsilon (1.0 / 1024)

#ifdef SHIZ_DEBUG
f64
z_time__get_lag(void);
#endif

void
z_engine__present_frame(void);

static inline
void
z_str_to_upper(char * string)
{
    char * ptr;

    for (ptr = string; *ptr; ptr++) {
        *ptr = (char)toupper(*ptr);
    }
}

/**
 * @brief Determine whether two floating point values are approximately equal.
 */
static inline
bool
z_fequal(f32 const a, f32 const b)
{
    return (fabsf(b - a) < SHIZEpsilon);
}

/**
 * @brief Linearly interpolate between two values.
 */
static inline
f32
z_lerp(f32 const a, f32 const b, f32 const t)
{
    return a * (1.0f - t) + b * t;
}

static inline
i32
z_random_int_range(i32 const min, i32 const max)
{
    return min + (rand() % ((max + 1) - min));
}

static inline
f32
z_random_float(void)
{
    return rand() / (RAND_MAX + 1.0f);
}

static inline
f32
z_random_float_range(f32 const min, f32 const max)
{
    return z_lerp(min, max, z_random_float());
}

#endif // internal_h
