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

#ifndef profiler_h
#define profiler_h

#include <SHIZEN/ztype.h>

#include <stdbool.h>

typedef struct SHIZProfilerStats {
    f64 frame_time;
    f64 frame_time_avg;
    u16 frames_per_second;
    u16 frames_per_second_min;
    u16 frames_per_second_max;
    u16 frames_per_second_avg;
    u16 draw_count;
    u8 _pad[6];
} SHIZProfilerStats;

bool
z_profiler__init(void);

bool
z_profiler__kill(void);

void
z_profiler__begin(void);

void
z_profiler__end(void);

void
z_profiler__increment_draw_count(u8 amount);

void
z_profiler__set_is_profiling(bool enabled);

bool
z_profiler__is_profiling(void);

SHIZProfilerStats
z_profiler__get_stats(void);

#endif /* profiler_h */
