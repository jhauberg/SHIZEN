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

#include <SHIZEN/ztime.h>
#include <SHIZEN/zmath.h>

#include <math.h> // fabs

#include "internal.h"

static SHIZTimeline const SHIZTimelineDefault = {
    .time = 0,
    .time_step = 0,
    .scale = 1
};

typedef struct SHIZTimelineState {
    double time_previous;
    double time_lag;
} SHIZTimelineState;

static double const maximum_frame_time = 1.0 / 4; // effectively 4 frames per second

static SHIZTimeline _timeline;
static SHIZTimelineState _timeline_state = {
    .time_previous = 0,
    .time_lag = 0
};

static bool _is_ticking = false;

void
z_time_reset()
{
    _timeline = SHIZTimelineDefault;
    _timeline_state.time_previous = _timeline.time;
    
    glfwSetTime(_timeline.time);
}

void
z_timing_begin()
{
    if (_is_ticking) {
        return;
    }

    _is_ticking = true;

    double const time = glfwGetTime();
    double time_elapsed = time - _timeline_state.time_previous;
    
    if (time_elapsed > maximum_frame_time) {
        time_elapsed = maximum_frame_time;
    }

    _timeline_state.time_lag += fabs(time_elapsed * _timeline.scale);
    _timeline_state.time_previous = time;
}

bool
z_time_tick(uint8_t const frequency)
{
    _timeline.time_step = 1.0 / frequency;
    
    if (_timeline_state.time_lag >= _timeline.time_step) {
        _timeline_state.time_lag -= _timeline.time_step;
        
        _timeline.time += _timeline.time_step * z_time_get_direction();
        
        return true;
    }
    
    return false;
}

double
z_timing_end()
{
    if (!_is_ticking) {
        return 0;
    }

    _is_ticking = false;

    double const interpolation = _timeline_state.time_lag / _timeline.time_step;

    return interpolation;
}

double
z_time_passed()
{
    return _timeline.time;
}

double
z_time_passed_since(double const time)
{
    double const time_passed_since = z_time_passed() - time;
    
    return time_passed_since;
}

double
z_time_get_scale()
{
    return _timeline.scale;
}

void
z_time_set_scale(double const scale)
{
    _timeline.scale = scale;
    
    if (z_fequal((float)_timeline.scale, 0)) {
        _timeline.scale = 0;
    }
}

double
z_time_get_tick_rate()
{
    return _timeline.time_step;
}

SHIZTimeDirection
z_time_get_direction()
{
    if (_timeline.scale > 0) {
        return SHIZTimeDirectionForward;
    } else if (_timeline.scale < 0) {
        return SHIZTimeDirectionBackward;
    }
    
    return SHIZTimeDirectionStill;
}

static
void
z_animate_time(double * time,
               double const step,
               double const duration)
{
    *time += step;
    
    if (*time > duration) {
        *time = duration;
    }
}

static
void
z_animate_value_to(float * next_value,
                   float const value,
                   float const to,
                   double const time,
                   double const duration)
{
    float const t = duration > 0 ? (float)(time / duration) : 1;
    
    float d = z_lerp(value, to, t);
    
    if (z_fequal(d, to)) {
        d = to;
    }
    
    *next_value = d;
}

static
float
z_animate_blend_value(float const previous,
                      float const next,
                      double const interpolation)
{
    if (z_fequal(previous, next)) {
        return next;
    }
    
    float t = (float)interpolation;
    
    if (t < 0) {
        t = 0;
    } else if (t > 1) {
        t = 1;
    }
    
    return z_lerp(previous, next, t);
}

void
z_animate(SHIZAnimatable * const animatable,
          float const to)
{
    animatable->value = to;
    animatable->result_prev = animatable->result;
    animatable->result = animatable->value;
}

void
z_animate_vec2(SHIZAnimatableVector2 * const animatable,
               SHIZVector2 const to)
{
    animatable->value = to;
    animatable->result_prev = animatable->result;
    animatable->result = animatable->value;
}

void
z_animate_to(SHIZAnimatable * const animatable,
             float const to,
             double const duration)
{
    if (duration > 0) {
        double const step = z_time_get_tick_rate();
        
        z_animate_time(&animatable->time, step, duration);
        
        animatable->result_prev = animatable->result;
        
        z_animate_value_to(&animatable->result,
                           animatable->value,
                           to,
                           animatable->time,
                           duration);
    } else {
        z_animate(animatable, to);
    }
}

void
z_animate_add(SHIZAnimatable * const animatable,
              float const add)
{
    double const step = z_time_get_tick_rate();
    
    float const from = animatable->value;
    float const to = from + (float)(add * step);
    
    z_animate(animatable, to);
}

void
z_animate_vec2_to(SHIZAnimatableVector2 * const animatable,
                  SHIZVector2 const to,
                  double const duration)
{
    if (duration > 0) {
        double const step = z_time_get_tick_rate();
        
        z_animate_time(&animatable->time, step, duration);
        
        animatable->result_prev = animatable->result;
        
        z_animate_value_to(&animatable->result.x,
                           animatable->value.x, to.x,
                           animatable->time,
                           duration);
        
        z_animate_value_to(&animatable->result.y,
                           animatable->value.y, to.y,
                           animatable->time,
                           duration);
    } else {
        z_animate_vec2(animatable, to);
    }
}

void
z_animate_vec2_add(SHIZAnimatableVector2 * const animatable,
                   SHIZVector2 const add)
{
    double const step = z_time_get_tick_rate();
    
    SHIZVector2 const from = animatable->value;
    SHIZVector2 const to = SHIZVector2Make(from.x + (float)(add.x * step),
                                           from.y + (float)(add.y * step));
    
    z_animate_vec2(animatable, to);
}

float
z_animate_blend(SHIZAnimatable * const animatable,
                double const interpolation)
{
    return z_animate_blend_value(animatable->result_prev,
                                 animatable->result,
                                 interpolation);
}

SHIZVector2
z_animate_vec2_blend(SHIZAnimatableVector2 * const animatable,
                     double const interpolation)
{
    float const x = z_animate_blend_value(animatable->result_prev.x,
                                        animatable->result.x,
                                        interpolation);
    float const y = z_animate_blend_value(animatable->result_prev.y,
                                        animatable->result.y,
                                        interpolation);
    
    return SHIZVector2Make(x, y);
}

#ifdef SHIZ_DEBUG
double
z_time__get_lag()
{
    return _timeline_state.time_lag;
}
#endif
