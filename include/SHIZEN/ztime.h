#pragma once

#include <stdbool.h> // bool

#include "ztype.h" // SHIZVector2

typedef enum SHIZTimeDirection {
    SHIZTimeDirectionBackward = -1,
    SHIZTimeDirectionStill = 0,
    SHIZTimeDirectionForward = 1
} SHIZTimeDirection;

void z_time_reset(void);

void z_timing_begin(void);
double z_timing_end(void);

bool z_time_tick(uint8_t frequency);

double z_time_passed_since(double time);
double z_time_passed(void);

double z_time_get_scale(void);
void z_time_set_scale(double scale);

double z_time_get_lag(void);
double z_time_get_tick_rate(void);

/**
 * @brief Determine the direction of passing time.
 *
 * The direction is ultimately determined by the time scale, and is always
 * one of three possible values: Backward, Still or Forward.
 */
SHIZTimeDirection z_time_get_direction(void);

typedef struct SHIZAnimatable {
    double time;
    float value;
    float result;
    float result_prev;
} SHIZAnimatable;

typedef struct SHIZAnimatableVector2 {
    double time;
    SHIZVector2 value;
    SHIZVector2 result;
    SHIZVector2 result_prev;
} SHIZAnimatableVector2;

void z_animate(SHIZAnimatable *, float to);
void z_animate_add(SHIZAnimatable *, float add);
void z_animate_to(SHIZAnimatable *, float to, double duration);

void z_animate_vec2(SHIZAnimatableVector2 *, SHIZVector2 to);
void z_animate_vec2_add(SHIZAnimatableVector2 *, SHIZVector2 add);
void z_animate_vec2_to(SHIZAnimatableVector2 *, SHIZVector2 to, double duration);

float z_animate_blend(SHIZAnimatable *, double interpolation);

SHIZVector2 z_animate_vec2_blend(SHIZAnimatableVector2 *, double interpolation);

static inline
SHIZAnimatable
SHIZAnimated(float const value)
{
    SHIZAnimatable animatable = {
        .time = 0,
        .value = value,
        .result = value,
        .result_prev = value
    };
    
    return animatable;
}

static inline
SHIZAnimatableVector2
SHIZAnimatedVector2(SHIZVector2 const value)
{
    SHIZAnimatableVector2 animatable = {
        .time = 0,
        .value = value,
        .result = value,
        .result_prev = value
    };
    
    return animatable;
}
