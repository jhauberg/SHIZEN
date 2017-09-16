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

#include <math.h>

#include "internal.h"
#include "internal_type.h"

static SHIZTimeline const SHIZTimelineDefault = {
    .time = 0,
    .time_step = 0,
    .scale = 1
};

typedef struct SHIZTimelineState {
    double time_previous;
    double time_lag;
    
    bool is_ticking;
} SHIZTimelineState;

static double const maximum_frame_time = 1.0 / 4; // effectively 4 frames per second

static SHIZTimeline _timeline;
static SHIZTimelineState _timeline_state = {
    .time_previous = 0,
    .time_lag = 0,
    .is_ticking = false
};

void
shiz_time_reset()
{
    _timeline = SHIZTimelineDefault;
    _timeline_state.time_previous = _timeline.time;
    
    glfwSetTime(_timeline.time);
}

double
shiz_time_since(double const time)
{
    double const time_passed_since = shiz_get_time() - time;
    
    return time_passed_since;
}

void
shiz_ticking_begin()
{
    if (_timeline_state.is_ticking) {
        return;
    }

    _timeline_state.is_ticking = true;

    double const time = glfwGetTime();
    double time_elapsed = time - _timeline_state.time_previous;
    
    if (time_elapsed > maximum_frame_time) {
        time_elapsed = maximum_frame_time;
    }

    _timeline_state.time_lag += fabs(time_elapsed * _timeline.scale);
    _timeline_state.time_previous = time;
}

bool
shiz_tick(unsigned short const frequency)
{
    _timeline.time_step = 1.0 / frequency;
    
    if (_timeline_state.time_lag >= _timeline.time_step) {
        _timeline_state.time_lag -= _timeline.time_step;
        
        SHIZTimeDirection const direction = shiz_get_time_direction();
        
        _timeline.time += _timeline.time_step * direction;
        
        return true;
    }
    
    return false;
}

double
shiz_ticking_end()
{
    if (!_timeline_state.is_ticking) {
        return 0;
    }

    _timeline_state.is_ticking = false;

    double const interpolation = _timeline_state.time_lag / _timeline.time_step;

    return interpolation;
}

double
shiz_get_tick_rate()
{
    return _timeline.time_step;
}

double
shiz_get_time_lag()
{
    return _timeline_state.time_lag;
}

double
shiz_get_time()
{
    return _timeline.time;
}

double
shiz_get_time_scale()
{
    return _timeline.scale;
}

void
shiz_set_time_scale(double const scale)
{
    _timeline.scale = scale;
    
    if (_shiz_fequal(_timeline.scale, 0)) {
        _timeline.scale = 0;
    }
}

SHIZTimeDirection
shiz_get_time_direction()
{
    if (_timeline.scale > 0) {
        return SHIZTimeDirectionForward;
    } else if (_timeline.scale < 0) {
        return SHIZTimeDirectionBackward;
    }
    
    return SHIZTimeDirectionStill;
}

void shiz_animate(SHIZAnimatable * const animatable,
                  double const interpolation)
{
    animatable->previous_result = animatable->result;
    animatable->result = _shiz_lerp(animatable->value,
                                    animatable->previous_result,
                                    interpolation);
}
