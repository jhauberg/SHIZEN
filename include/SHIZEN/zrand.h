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

#pragma once

#include <SHIZEN/ztype.h> // SHIZColor

#include <stdint.h> // uint32_t, int32_t
#include <stdbool.h> // bool

void z_rand_seed(uint32_t seed);
void z_rand_seed_now(void);

uint32_t z_rand_get_seed(void);

uint32_t z_rand(void);
int32_t z_rand_range(int32_t const min,
                     int32_t const max);

float z_randf(void);
float z_randf_range(float const min, float const max);
float z_randf_angle(void);

SHIZColor z_rand_color(void);

bool z_rand_choice(void);
bool z_rand_chance(float percentage);
