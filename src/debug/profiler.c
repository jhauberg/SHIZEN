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

#include "profiler.h"

#include "../internal.h"

static
void
z_profiler__update_averages(void);

static double const _frame_average_interval = 1.0; // in seconds

static double _last_frame_time = 0;
static double _last_average_time = 0;

static uint16_t _frame_samples = 0; // sample frames to calculate average
static uint16_t _frame_sample_count = 0;

static double _frame_time = 0;
static double _frame_time_avg = 0;

static double _frame_time_samples = 0; // sample frames to calculate average
static uint16_t _frame_time_sample_count = 0;

static SHIZProfilerStats _stats;

static bool _is_profiling = true;

bool
z_profiler__init()
{
    _stats.draw_count = 0;
    _stats.frame_time = 0;
    _stats.frame_time_avg = 0;
    _stats.frames_per_second = 0;
    _stats.frames_per_second_min = UINT16_MAX;
    _stats.frames_per_second_max = 0;
    _stats.frames_per_second_avg = 0;
    
    return true;
}

bool
z_profiler__kill()
{
    return true;
}

void
z_profiler__begin()
{
    _stats.draw_count = 0;
}

void
z_profiler__increment_draw_count(uint8_t const amount)
{
    if (_is_profiling) {
        _stats.draw_count += amount;
    }
}

void
z_profiler__end()
{
    double const time = glfwGetTime();
    double const time_since_last_frame = time - _last_frame_time;
    
    _frame_time = time_since_last_frame;
    _last_frame_time = time;
    
    _stats.frames_per_second = (uint16_t)(1.0 / _frame_time);
    
    if (_stats.frames_per_second < _stats.frames_per_second_min) {
        _stats.frames_per_second_min = _stats.frames_per_second;
    }
    
    if (_stats.frames_per_second > _stats.frames_per_second_max) {
        _stats.frames_per_second_max = _stats.frames_per_second;
    }
    
    _frame_samples += _stats.frames_per_second;
    _frame_sample_count++;
    
    _frame_time_samples += _frame_time;
    _frame_time_sample_count++;
    
    double const time_since_last_average = time - _last_average_time;
    
    if (time_since_last_average >= _frame_average_interval) {
        _last_average_time = time;
        
        z_profiler__update_averages();
    }
    
    _stats.frame_time = _frame_time * 1000;
    _stats.frame_time_avg = _frame_time_avg * 1000;
}

void
z_profiler__set_is_profiling(bool const enabled)
{
    _is_profiling = enabled;
}

bool
z_profiler__is_profiling()
{
    return _is_profiling;
}

SHIZProfilerStats
z_profiler__get_stats()
{
    return _stats;
}

static
void
z_profiler__update_averages()
{
    _stats.frames_per_second_avg = _frame_samples / _frame_sample_count;
    _frame_sample_count = 0;
    _frame_samples = 0;
    
    _frame_time_avg = _frame_time_samples / _frame_time_sample_count;
    _frame_time_sample_count = 0;
    _frame_time_samples = 0;
    
    // reset min/max to show rolling stats rather than historically accurate stats (its more interesting
    // knowing min/max for the current scene/context than knowing the 9999+ max fps during the first blank frame)
    _stats.frames_per_second_min = UINT16_MAX;
    _stats.frames_per_second_max = 0;
}
