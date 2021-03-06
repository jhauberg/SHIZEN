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

#include <stdbool.h> // bool
#include <stdint.h> // uint8_t, int16_t, int32_t

#include "res.h" // SHIZResourceSound

bool z_mixer__init(void);
bool z_mixer__kill(void);

void z_mixer__play_sound(uint8_t sound_resource_id);
void z_mixer__stop_sound(uint8_t sound_resource_id);

bool z_mixer__create_sound(SHIZResourceSound * resource,
                           int32_t channels,
                           int32_t sample_rate,
                           int16_t * data,
                           int32_t size);

bool z_mixer__destroy_sound(SHIZResourceSound const * resource);
