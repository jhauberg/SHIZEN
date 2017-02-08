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

#ifndef gfx_debug_h
#define gfx_debug_h

#include "debug.h"

void shiz_gfx_debug_reset_draw_count(void);
void shiz_gfx_debug_increment_draw_count(unsigned int amount);
void shiz_gfx_debug_update_frame_stats(SHIZDebugFrameStats * stats);

#endif // gfx_debug_h
