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

#ifndef draw_debug_h
#define draw_debug_h

#ifdef SHIZ_DEBUG

#include "../internal.h"

void
z_debug__build_stats(void);

void
z_debug__draw_stats(void);

void
z_debug__draw_events(void);

void
z_debug__draw_viewport(void);

void
z_debug__draw_points_bounds(SHIZVector2 const points[], uint16_t count,
                            SHIZColor color,
                            float angle,
                            SHIZLayer layer);

void
z_debug__draw_circle_bounds(SHIZVector2 origin,
                            SHIZColor color,
                            float radius,
                            SHIZVector2 scale,
                            SHIZLayer layer);

void
z_debug__draw_rect_bounds(SHIZRect rect,
                          SHIZColor color,
                          SHIZVector2 anchor,
                          float angle,
                          SHIZLayer layer);

void
z_debug__draw_sprite_bounds(SHIZVector2 origin,
                            SHIZSize size,
                            SHIZColor color,
                            SHIZVector2 anchor,
                            float angle,
                            SHIZLayer layer);

#endif

#endif /* draw_debug_h */
