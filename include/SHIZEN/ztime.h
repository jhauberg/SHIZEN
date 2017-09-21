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

#ifndef ztime_h
#define ztime_h

#include <stdbool.h>

#include "zint.h"

typedef enum SHIZTimeDirection {
    SHIZTimeDirectionBackward = -1,
    SHIZTimeDirectionStill = 0,
    SHIZTimeDirectionForward = 1
} SHIZTimeDirection;

void
z_time_reset(void);

void
z_timing_begin(void);

f64
z_timing_end(void);

bool
z_time_tick(u8 frequency);

f64
z_time_passed_since(f64 time);

f64
z_time_passed(void);

f64
z_time_get_scale(void);

void
z_time_set_scale(f64 scale);

f64
z_time_get_tick_rate(void);

/**
 * @brief Determine the direction of passing time.
 *
 * The direction is ultimately determined by the time scale, and is always
 * one of three possible values: Backward, Still or Forward.
 */
SHIZTimeDirection
z_time_get_direction(void);

typedef struct SHIZAnimatable {
    f32 value;
    f32 previous_result;
    f32 result;
} SHIZAnimatable;

void
z_animate(SHIZAnimatable *,
          f64 interpolation);

static inline
SHIZAnimatable
SHIZAnimated(f32 const value)
{
    SHIZAnimatable animatable = {
        .value = value,
        .previous_result = 0,
        .result = 0
    };
    
    z_animate(&animatable, 1);
    
    return animatable;
}

#endif // ztime_h
