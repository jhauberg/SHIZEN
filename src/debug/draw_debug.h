#pragma once

#ifdef SHIZ_DEBUG

#include "../internal.h" // SHIZ*

#include <stdint.h> // uint16_t

void z_debug__build_stats(void);
void z_debug__draw_stats(void);

void z_debug__draw_viewport(void);

void z_debug__draw_points_bounds(SHIZVector2 const points[], uint16_t count,
                                 SHIZColor color,
                                 float angle,
                                 SHIZLayer layer);

void z_debug__draw_circle_bounds(SHIZVector2 origin,
                                 SHIZColor color,
                                 float radius,
                                 SHIZVector2 scale,
                                 SHIZLayer layer);

void z_debug__draw_rect_bounds(SHIZRect rect,
                               SHIZColor color,
                               SHIZVector2 anchor,
                               float angle,
                               SHIZLayer layer);

void z_debug__draw_sprite_bounds(SHIZVector2 origin,
                                 SHIZSize size,
                                 SHIZColor color,
                                 SHIZVector2 anchor,
                                 float angle,
                                 SHIZLayer layer);

#endif
