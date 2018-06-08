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

#include <stdbool.h> // bool

#include <math.h> // fabsf

/**
 * @brief Determine whether two floating point values are approximately equal.
 */
static inline
bool
z_fequal(float const a, float const b)
{
    static double const precision = 1.0 / 2048;
    
    return fabsf(b - a) < precision;
}

/**
 * @brief Linearly interpolate between two values.
 */
static inline
float
z_lerp(float const a, float const b, float const t)
{
    return (1.0f - t) * a + t * b;
}

static inline
double
z_lerpf(double const a, double const b, double const t)
{
    return (1.0 - t) * a + t * b;
}
