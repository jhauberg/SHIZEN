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

#ifndef mixer_h
#define mixer_h

#include <stdbool.h>

#include <SHIZEN/ztype.h>

#include "res.h"

bool
z_mixer__init(void);

bool
z_mixer__kill(void);

void
z_mixer__play_sound(u8 sound);
void
z_mixer__stop_sound(u8 sound);

bool
z_mixer__create_sound(SHIZResourceSound * resource,
                      int channels,
                      int sample_rate,
                      short * data,
                      int size);
bool
z_mixer__destroy_sound(SHIZResourceSound const * resource);

#endif /* mixer_h */
