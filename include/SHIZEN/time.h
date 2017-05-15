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

#ifndef time_h
#define time_h

#include <stdbool.h>

void shiz_time_reset(void);

void shiz_ticking_begin(void);
double shiz_ticking_end(void);

bool shiz_tick(unsigned short const frequency);

double shiz_get_tick_rate(void);
double shiz_get_time_lag(void);
double shiz_get_time(void);
double shiz_get_time_scale(void);

void shiz_set_time_scale(double const scale);

float shiz_get_time_direction(void);

float shiz_animate(float const value,
                   float const previous_value,
                   double const interpolation);

#endif // time_h
