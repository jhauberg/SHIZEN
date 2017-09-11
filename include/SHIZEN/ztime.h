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

#ifdef ztime_h
#error this header is already defined
#endif

#ifndef ztime_h
#define ztime_h

#include <stdbool.h>

typedef enum SHIZTimeDirection {
    SHIZTimeDirectionBackward = -1,
    SHIZTimeDirectionStill = 0,
    SHIZTimeDirectionForward = 1
} SHIZTimeDirection;

void shiz_time_reset(void);

double shiz_time_since(double time);

void shiz_ticking_begin(void);
double shiz_ticking_end(void);

bool shiz_tick(unsigned short frequency);

double shiz_get_tick_rate(void);
double shiz_get_time_lag(void);
double shiz_get_time(void);
double shiz_get_time_scale(void);

void shiz_set_time_scale(double scale);

/**
 * @brief Determine the direction of passing time.
 *
 * The direction is ultimately determined by the time scale, and is always
 * one of three possible values: Backward, Still or Forward.
 */
SHIZTimeDirection shiz_get_time_direction(void);

typedef struct SHIZAnimatable {
    float value;
    float previous_result;
    float result;
} SHIZAnimatable;

void shiz_animate(SHIZAnimatable *, double interpolation);

static inline
SHIZAnimatable
SHIZAnimated(float value)
{
    SHIZAnimatable animatable = {
        .value = value,
        .previous_result = 0,
        .result = 0
    };
    
    shiz_animate(&animatable, 1);
    
    return animatable;
}

#endif // time_h
