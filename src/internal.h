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
#define SHIZEN_VERSION_MINOR 18
#define SHIZEN_VERSION_PATCH 2

#define SHIZEN_VERSION_NAME "ALPHA"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#define SHIZEpsilon (1.0 / 1024)

void _shiz_present_frame(void);

static inline
void
_shiz_str_to_upper(char * string)
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
_shiz_fequal(double const a, double const b)
{
    return (fabs(b - a) < SHIZEpsilon);
}

static inline
float
_shiz_lerp(float const a, float const b, float const t)
{
    return a * (1.0f - t) + b * t;
}

static inline
int
_shiz_random_int_range(int const min, int const max)
{
    return min + (rand() % (int)((max + 1) - min));
}

static inline
float
_shiz_random_float(void)
{
    return rand() / (RAND_MAX + 1.0f);
}

static inline
float
_shiz_random_float_range(float const min, float const max)
{
    return _shiz_lerp(min, max, _shiz_random_float());
}

#endif // internal_h
