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

#include <SHIZEN/zsound.h>

#include "mixer.h"

void
z_sound_play(u8 const sound)
{
    z_mixer__play_sound(sound);
}

void
z_sound_stop(u8 const sound)
{
    z_mixer__stop_sound(sound);
}
