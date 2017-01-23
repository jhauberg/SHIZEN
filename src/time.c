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

#include <SHIZEN/time.h>

#include <math.h>

#include "internal.h"

static double const maximum_frame_time = 1.0 / 4; // effectively 4 frames per second

static SHIZTimeLine _timeline;

static double _time_previous = 0;
static double _time_lag = 0;

static bool _is_ticking = false;

void shiz_time_reset() {
    _timeline = SHIZTimeLineDefault;
    
    glfwSetTime(_timeline.time);
    
    _time_previous = glfwGetTime();
}

void shiz_ticking_begin() {
    if (_is_ticking) {
        return;
    }

    _is_ticking = true;

    double const time = glfwGetTime();
    double time_elapsed = time - _time_previous;
    
    if (time_elapsed > maximum_frame_time) {
        time_elapsed = maximum_frame_time;
    }
    
    _time_lag += fabs(time_elapsed * _timeline.scale);
    _time_previous = time;
}

bool shiz_tick(ushort const frequency) {
    _timeline.time_step = 1.0 / frequency;
    
    if (_time_lag >= _timeline.time_step) {
        _time_lag -= _timeline.time_step;
        
        _timeline.time += _timeline.time_step * shiz_get_time_direction();
        
        return true;
    }
    
    return false;
}

double shiz_ticking_end() {
    if (!_is_ticking) {
        return 0;
    }

    _is_ticking = false;

    return _time_lag / _timeline.time_step;
}

double shiz_get_tick_rate() {
    return _timeline.time_step;
}

double shiz_get_time() {
    return _timeline.time;
}

double shiz_get_time_scale() {
    return _timeline.scale;
}

void shiz_set_time_scale(double const scale) {
    _timeline.scale = scale;
}

float shiz_get_time_direction() {
    if (_timeline.scale > 0) {
        return 1;
    } else if (_timeline.scale < 0) {
        return -1;
    }
    
    return 0;
}
