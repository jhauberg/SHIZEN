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

#include <math.h> // fabs

#include "internal.h"
#include "internal_type.h"

static SHIZTimeline const SHIZTimelineDefault = {
    .time = 0,
    .time_step = 0,
    .scale = 1
};

typedef struct SHIZTimelineState {
    f64 time_previous;
    f64 time_lag;
} SHIZTimelineState;

static f64 const maximum_frame_time = 1.0 / 4; // effectively 4 frames per second

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

    f64 const time = glfwGetTime();
    f64 time_elapsed = time - _timeline_state.time_previous;
    
    if (time_elapsed > maximum_frame_time) {
        time_elapsed = maximum_frame_time;
    }

    _timeline_state.time_lag += fabs(time_elapsed * _timeline.scale);
    _timeline_state.time_previous = time;
}

bool
z_time_tick(u8 const frequency)
{
    _timeline.time_step = 1.0 / frequency;
    
    if (_timeline_state.time_lag >= _timeline.time_step) {
        _timeline_state.time_lag -= _timeline.time_step;
        
        _timeline.time += _timeline.time_step * z_time_get_direction();
        
        return true;
    }
    
    return false;
}

f64
z_timing_end()
{
    if (!_is_ticking) {
        return 0;
    }

    _is_ticking = false;

    f64 const interpolation = _timeline_state.time_lag / _timeline.time_step;

    return interpolation;
}

f64
z_time_passed()
{
    return _timeline.time;
}

f64
z_time_passed_since(f64 const time)
{
    f64 const time_passed_since = z_time_passed() - time;
    
    return time_passed_since;
}

f64
z_time_get_scale()
{
    return _timeline.scale;
}

void
z_time_set_scale(f64 const scale)
{
    _timeline.scale = scale;
    
    if (z_fequal((f32)_timeline.scale, 0)) {
        _timeline.scale = 0;
    }
}

f64
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
z_animate_time(f64 * time,
               f64 const step,
               f64 const duration)
{
    *time += step;
    
    if (*time > duration) {
        *time = duration;
    }
}

static
void
z_animate_value_to(f32 * next_value,
                   f32 const value,
                   f32 const to,
                   f64 const time,
                   f64 const duration)
{
    f32 const t = duration > 0 ? (f32)(time / duration) : 1;
    
    f32 d = z_lerp(value, to, t);
    
    if (z_fequal(d, to)) {
        d = to;
    }
    
    *next_value = d;
}

static
f32
z_animate_blend_value(f32 const previous,
                      f32 const next,
                      f64 const interpolation)
{
    if (z_fequal(previous, next)) {
        return next;
    }
    
    f32 t = (f32)interpolation;
    
    if (t < 0) {
        t = 0;
    } else if (t > 1) {
        t = 1;
    }
    
    return z_lerp(previous, next, t);
}

void
z_animate(SHIZAnimatable * const animatable,
          f32 const to)
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
             f32 const to,
             f64 const duration)
{
    if (duration > 0) {
        f64 const step = z_time_get_tick_rate();
        
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
              f32 const add)
{
    f64 const step = z_time_get_tick_rate();
    
    f32 const from = animatable->value;
    f32 const to = from + (f32)(add * step);
    
    z_animate(animatable, to);
}

void
z_animate_vec2_to(SHIZAnimatableVector2 * const animatable,
                  SHIZVector2 const to,
                  f64 const duration)
{
    if (duration > 0) {
        f64 const step = z_time_get_tick_rate();
        
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
    f64 const step = z_time_get_tick_rate();
    
    SHIZVector2 const from = animatable->value;
    SHIZVector2 const to = SHIZVector2Make(from.x + (f32)(add.x * step),
                                           from.y + (f32)(add.y * step));
    
    z_animate_vec2(animatable, to);
}

f32
z_animate_blend(SHIZAnimatable * const animatable,
                f64 const interpolation)
{
    return z_animate_blend_value(animatable->result_prev,
                                 animatable->result,
                                 interpolation);
}

SHIZVector2
z_animate_vec2_blend(SHIZAnimatableVector2 * const animatable,
                     f64 const interpolation)
{
    f32 const x = z_animate_blend_value(animatable->result_prev.x,
                                        animatable->result.x,
                                        interpolation);
    f32 const y = z_animate_blend_value(animatable->result_prev.y,
                                        animatable->result.y,
                                        interpolation);
    
    return SHIZVector2Make(x, y);
}

#ifdef SHIZ_DEBUG
f64
z_time__get_lag()
{
    return _timeline_state.time_lag;
}
#endif
