#pragma once

//#include <SHIZEN/ztype.h>

#include <stdint.h> // uint8_t, uint16_t
#include <stdbool.h> // bool

typedef struct SHIZProfilerStats {
    double frame_time;
    double frame_time_avg;
    uint16_t frames_per_second;
    uint16_t frames_per_second_min;
    uint16_t frames_per_second_max;
    uint16_t frames_per_second_avg;
    uint16_t draw_count;
} SHIZProfilerStats;

bool z_profiler__init(void);
bool z_profiler__kill(void);

void z_profiler__begin(void);
void z_profiler__end(void);

void z_profiler__increment_draw_count(uint8_t amount);

void z_profiler__set_is_profiling(bool enabled);
bool z_profiler__is_profiling(void);

SHIZProfilerStats z_profiler__get_stats(void);
