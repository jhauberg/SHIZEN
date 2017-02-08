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

#ifdef SHIZ_DEBUG
#include "gfx.debug.h"

#include "internal.h"

#include <limits.h>

static void _shiz_gfx_debug_update_frame_averages(void);

static unsigned int _shiz_gfx_debug_draw_count = 0;

static double const _shiz_gfx_debug_average_interval = 1.0; // in seconds
static double _shiz_gfx_debug_last_average_time = 0;

static unsigned int _shiz_gfx_debug_frame_samples = 0; // sample frames to calculate average
static unsigned int _shiz_gfx_debug_frame_sample_count = 0;

static unsigned int _shiz_gfx_debug_frames_per_second = 0;
static unsigned int _shiz_gfx_debug_frames_per_second_min = UINT_MAX;
static unsigned int _shiz_gfx_debug_frames_per_second_max = 0;
static unsigned int _shiz_gfx_debug_frames_per_second_avg = 0;

static double _shiz_gfx_debug_frame_time = 0;
static double _shiz_gfx_debug_frame_time_avg = 0;

static double _shiz_gfx_debug_frame_time_samples = 0; // sample frames to calculate average
static unsigned int _shiz_gfx_debug_frame_time_sample_count = 0;

static double _shiz_gfx_debug_last_frame_time = 0;

void
shiz_gfx_debug_reset_draw_count()
{
    _shiz_gfx_debug_draw_count = 0;
}

void
shiz_gfx_debug_increment_draw_count(unsigned int amount)
{
    if (shiz_debug_is_events_enabled()) {
        _shiz_gfx_debug_draw_count += amount;
    }
}

void
shiz_gfx_debug_update_frame_stats(SHIZDebugFrameStats * stats)
{
    double const time = glfwGetTime();
    double const time_since_last_frame = time - _shiz_gfx_debug_last_frame_time;
    
    _shiz_gfx_debug_frame_time = time_since_last_frame;
    _shiz_gfx_debug_last_frame_time = time;
    
    _shiz_gfx_debug_frames_per_second = 1.0 / _shiz_gfx_debug_frame_time;
    
    if (_shiz_gfx_debug_frames_per_second < _shiz_gfx_debug_frames_per_second_min) {
        _shiz_gfx_debug_frames_per_second_min = _shiz_gfx_debug_frames_per_second;
    }
    
    if (_shiz_gfx_debug_frames_per_second > _shiz_gfx_debug_frames_per_second_max) {
        _shiz_gfx_debug_frames_per_second_max = _shiz_gfx_debug_frames_per_second;
    }
    
    _shiz_gfx_debug_frame_samples += _shiz_gfx_debug_frames_per_second;
    _shiz_gfx_debug_frame_sample_count++;
    
    _shiz_gfx_debug_frame_time_samples += _shiz_gfx_debug_frame_time;
    _shiz_gfx_debug_frame_time_sample_count++;
    
    double const time_since_last_average = time - _shiz_gfx_debug_last_average_time;
    
    if (time_since_last_average >= _shiz_gfx_debug_average_interval) {
        _shiz_gfx_debug_last_average_time = time;
        
        _shiz_gfx_debug_update_frame_averages();
    }
    
    stats->draw_count = _shiz_gfx_debug_draw_count;
    stats->frame_time = _shiz_gfx_debug_frame_time * 1000;
    stats->frame_time_avg = _shiz_gfx_debug_frame_time_avg * 1000;
    stats->frames_per_second = _shiz_gfx_debug_frames_per_second;
    stats->frames_per_second_min = _shiz_gfx_debug_frames_per_second_min;
    stats->frames_per_second_max = _shiz_gfx_debug_frames_per_second_max;
    stats->frames_per_second_avg = _shiz_gfx_debug_frames_per_second_avg;
}

static void
_shiz_gfx_debug_update_frame_averages()
{
    _shiz_gfx_debug_frames_per_second_avg = _shiz_gfx_debug_frame_samples / _shiz_gfx_debug_frame_sample_count;
    _shiz_gfx_debug_frame_sample_count = 0;
    _shiz_gfx_debug_frame_samples = 0;
    
    _shiz_gfx_debug_frame_time_avg = _shiz_gfx_debug_frame_time_samples / _shiz_gfx_debug_frame_time_sample_count;
    _shiz_gfx_debug_frame_time_sample_count = 0;
    _shiz_gfx_debug_frame_time_samples = 0;
    
    // reset min/max to show rolling stats rather than historically accurate stats (its more interesting
    // knowing min/max for the current scene/context than knowing the 9999+ max fps during the first blank frame)
    _shiz_gfx_debug_frames_per_second_min = UINT_MAX;
    _shiz_gfx_debug_frames_per_second_max = 0;
}
#endif
