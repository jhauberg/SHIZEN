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

#include <SHIZEN/zsound.h> // z_sound_*

#include <stdint.h> // uint8_t

#include "mixer.h" // z_mixer_*

void
z_sound_play(uint8_t const sound_resource_id)
{
    z_mixer__play_sound(sound_resource_id);
}

void
z_sound_stop(uint8_t const sound_resource_id)
{
    z_mixer__stop_sound(sound_resource_id);
}
