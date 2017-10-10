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

#ifndef recorder_h
#define recorder_h

#include <SHIZEN/ztype.h>

#include <stdbool.h>

bool
z_recorder__init(void);

bool
z_recorder__kill(void);

void
z_recorder__setup(SHIZSize resolution);

void
z_recorder__capture(void);

bool
z_recorder__start(void);

bool
z_recorder__stop(void);

bool
z_recorder_is_recording(void);

#endif /* recorder_h */
