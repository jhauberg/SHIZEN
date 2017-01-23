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
#include <stdint.h>

void shiz_time_reset(void);

void shiz_ticking_begin(void);
double shiz_ticking_end(void);

bool shiz_tick(ushort const frequency);

double shiz_get_tick_rate(void);
double shiz_get_time(void);
double shiz_get_time_scale(void);

void shiz_set_time_scale(double const scale);

float shiz_get_time_direction(void);

#endif // time_h
