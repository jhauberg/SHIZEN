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

#ifndef zint_h
#define zint_h

#include <stdint.h>
#include <stdbool.h>

#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

/**
 * @brief Determine whether two floating point values are approximately equal.
 */
static inline
bool
z_fequal(f32 const a, f32 const b)
{
    static f64 const precision = 1.0 / 1024;
    
    return fabsf(b - a) < precision;
}

/**
 * @brief Linearly interpolate between two values.
 */
static inline
f32
z_lerp(f32 const a, f32 const b, f32 const t)
{
    return (1.0f - t) * a + t * b;
}

static inline
f64
z_lerpf(f64 const a, f64 const b, f64 const t)
{
    return (1.0 - t) * a + t * b;
}

#endif /* zint_h */
