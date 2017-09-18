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

#ifdef SHIZ_DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SHIZEN/ztime.h>
#include <SHIZEN/zdraw.h>

#include "debug.h"

#include "../internal.h"
#include "../sprite.h"
#include "../spritefont.h"
#include "../viewport.h"
#include "../res.h"

extern SHIZGraphicsContext const _graphics_context;

static char _shiz_debug_stats_buffer[256] = { 0 };

void
z_debug__build_stats()
{
    SHIZViewport const viewport = z_viewport__get();
    
    unsigned int const sprite_count = z_debug__get_sprite_count();
    
    char const sprite_count_tint_specifier =
        sprite_count > SHIZSpriteMax ? '\3' : '\2';
    
    char display_size_buffer[32] = { 0 };
    
    if (viewport.resolution.width != viewport.framebuffer.width ||
        viewport.resolution.height != viewport.framebuffer.height) {
        sprintf(display_size_buffer, "\4%.0fx%.0f\1@\5%.0fx%.0f\1",
                viewport.resolution.width, viewport.resolution.height,
                viewport.framebuffer.width, viewport.framebuffer.height);
    } else {
        sprintf(display_size_buffer, "\4%.0fx%.0f\1",
                viewport.resolution.width, viewport.resolution.height);
    }
    
    SHIZDebugFrameStats const frame_stats = z_debug__get_frame_stats();
    
    bool const is_vsync_enabled = _graphics_context.swap_interval > 0;
    
    if (z_debug__is_expanded()) {
        sprintf(_shiz_debug_stats_buffer,
                "%s\n"
                "─────\n\n"
                "\2%0.2fms/frame\1 (\4%0.2fms\1)\n"
                "\2%d fps\1 (\3%d↓\1 \4%d↕\1 \5%d↑\1%s)\n\n"
                "%c%d/%d sprites/frame\1\n"
                "\2%d draws/frame\1\n\n"
                "\4%0.2fms\1/\2%0.2fms/tick\1\n"
                "\2%.1fx time\1",
                display_size_buffer,
                frame_stats.frame_time,
                frame_stats.frame_time_avg,
                frame_stats.frames_per_second,
                frame_stats.frames_per_second_min,
                frame_stats.frames_per_second_avg,
                frame_stats.frames_per_second_max,
                is_vsync_enabled ? " \2V\1" : "",
                sprite_count_tint_specifier, sprite_count, SHIZSpriteMax,
                frame_stats.draw_count,
                z_time__get_lag() * 1000,
                z_time__get_tick_rate() * 1000,
                z_time_get_scale());
    } else {
        sprintf(_shiz_debug_stats_buffer,
                "\2%d fps%s\1",
                frame_stats.frames_per_second,
                is_vsync_enabled ? " (V)" : "");
    }
}

void
z_debug__draw_events()
{
    unsigned int const line_margin = 8;
    unsigned int const lane_margin = 4;
    unsigned int const lane_size = 8;
    
    unsigned int draw_events = 0;
    
    SHIZLayer const layer = SHIZLayerTop;
    
    SHIZSize const bounds = _graphics_context.native_size;
    
    for (unsigned int i = 0; i < z_debug__get_event_count(); i++) {
        SHIZDebugEvent const event = z_debug__get_event(i);
        
        float const lane_offset =
        lane_margin + (lane_size * event.lane) + (lane_margin * event.lane);
        
        SHIZVector2 const from = SHIZVector2Make(event.origin.x,
                                                 bounds.height - lane_offset);
        SHIZVector2 const to = SHIZVector2Make(event.origin.x,
                                               event.origin.y);
        
        SHIZColor tint = SHIZSpriteNoTint;
        
        char event_buffer[32] = { 0 };
        
        if (event.lane == SHIZDebugEventLaneDraws) {
            tint = SHIZColorYellow;
            
            draw_events += 1;
            
            sprintf(event_buffer, "%u:%s", draw_events, event.name);
        } else if (event.lane == SHIZDebugEventLaneResources) {
            sprintf(event_buffer, "%s", event.name);
        }
        
        z_str_to_upper(event_buffer);
        
        z_draw_line_ex(SHIZVector2Make(from.x,
                                       from.y - line_margin - lane_margin),
                       to, tint, layer);
        
        SHIZSize const event_text_size =
        z_draw_text_ex(z_debug__get_font(),
                       event_buffer,
                       from,
                       SHIZSpriteFontAlignmentCenter|SHIZSpriteFontAlignmentTop,
                       SHIZSpriteFontSizeToFit, tint,
                       SHIZSpriteFontAttributesWithScale(1), layer);
        
        z_draw_rect_ex(SHIZRectMake(from, event_text_size),
                       SHIZColorBlack,
                       SHIZDrawModeFill,
                       SHIZAnchorTop,
                       SHIZSpriteNoAngle,
                       SHIZLayeredBelow(layer));
    }
}

void
z_debug__draw_stats()
{
    SHIZLayer const layer = SHIZLayeredBelow(SHIZLayerTop);
    
    SHIZSpriteFont const spritefont = z_debug__get_font();
    
    unsigned int const margin = 8;
    
    SHIZColor highlight_colors[] = {
        SHIZColorFromHex(0xefec0d), // yellow
        SHIZColorFromHex(0xe5152d), // red
        SHIZColorFromHex(0x36cd33), // green
        SHIZColorFromHex(0x20b1fc)  // blue
    };
    
    SHIZSize const bounds = _graphics_context.native_size;
    
    SHIZVector2 stats_text_origin =
    SHIZVector2Make(bounds.width - margin,
                    bounds.height - margin);
    
    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault; {
        attrs.character_spread = SHIZSpriteFontSpreadTight;
    }
    
    z_draw_text_ex_colored(spritefont,
                           _shiz_debug_stats_buffer,
                           stats_text_origin,
                           SHIZSpriteFontAlignmentTop|SHIZSpriteFontAlignmentRight,
                           SHIZSpriteFontSizeToFit, SHIZSpriteNoTint,
                           attrs, layer,
                           highlight_colors, 4);
    
    char version_buffer[128] = { 0 };
    
    sprintf(version_buffer, "SHIZEN %d.%d.%d / %s (%s)",
            SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
            SHIZEN_VERSION_NAME, __DATE__);
    
    z_draw_text_ex(spritefont,
                   version_buffer,
                   SHIZVector2Make(bounds.width - margin / 2,
                                   margin / 2),
                   SHIZSpriteFontAlignmentBottom|SHIZSpriteFontAlignmentRight,
                   SHIZSpriteFontSizeToFit,
                   SHIZSpriteTintDefaultWithAlpa(0.15f),
                   attrs, layer);
}

void
z_debug__draw_viewport()
{
    SHIZLayer const bounds_layer = SHIZLayerTop;
    
    SHIZViewport const viewport = z_viewport__get();
    
    SHIZVector2 const center = SHIZVector2Make(viewport.resolution.width / 2,
                                               viewport.resolution.height / 2);
    
    SHIZColor const bounds_color = SHIZColorWithAlpa(SHIZColorYellow, 0.8f);
    
    SHIZColor const x_color = SHIZColorWithAlpa(SHIZColorFromHex(0x419fff), 0.6f);
    SHIZColor const y_color = SHIZColorWithAlpa(SHIZColorGreen, 0.6f);
    
    SHIZRect const viewport_shape =
    SHIZRectMake(center, SHIZSizeMake(viewport.resolution.width - 1,
                                      viewport.resolution.height - 1));
    
    // bounds
    z_draw_rect_ex(viewport_shape, bounds_color,
                   SHIZDrawModeOutline, SHIZAnchorCenter, 0, bounds_layer);
    
    // axes
    SHIZLayer const axes_layer = SHIZLayeredBelow(SHIZLayerTop);
    
    float const padding = 2;
    float const text_padding = 2;
    
    SHIZVector2 const y_bottom =
    SHIZVector2Make(center.x, padding + text_padding);
    SHIZVector2 const y_top =
    SHIZVector2Make(center.x,
                    viewport.resolution.height - (padding + text_padding));
    
    SHIZVector2 const x_left =
    SHIZVector2Make(padding + text_padding, center.y);
    SHIZVector2 const x_right =
    SHIZVector2Make(viewport.resolution.width - (padding + text_padding),
                    center.y);
    
    z_draw_line_ex(y_top, y_bottom, y_color, axes_layer);
    z_draw_line_ex(x_left, x_right, x_color, axes_layer);
    
    SHIZSpriteFont const spritefont = z_debug__get_font();
    
    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault; {
        attrs.character_spread = SHIZSpriteFontSpreadTight;
    }
    
    SHIZColor point_color = bounds_color;
    
    if (z_debug__is_expanded()) {
        z_draw_text_ex(spritefont, "0,0",
                       SHIZVector2Make(spritefont.character.width / 2,
                                       (spritefont.character.height / 2) - 1),
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                       SHIZSpriteFontSizeToFit,
                       point_color, attrs, axes_layer);
        
        char center_text[32] = { 0 };
        
        sprintf(center_text, "%.0f,%.0f", center.x, center.y);
        
        z_draw_text_ex(spritefont, center_text,
                       SHIZVector2Make(center.x + spritefont.character.width / 2,
                                       center.y + (spritefont.character.height / 2) - 1),
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                       SHIZSpriteFontSizeToFit,
                       point_color, attrs, axes_layer);
        
        char y_max[32] = { 0 };
        char x_max[32] = { 0 };
        
        sprintf(y_max, "Y=%.0f", viewport.resolution.height);
        sprintf(x_max, "X=%.0f", viewport.resolution.width);
        
        z_draw_text_ex(spritefont, y_max,
                       SHIZVector2Make(y_top.x - spritefont.character.width / 2,
                                       y_top.y),
                       SHIZSpriteFontAlignmentRight|SHIZSpriteFontAlignmentTop,
                       SHIZSpriteFontSizeToFit,
                       y_color, attrs, axes_layer);
        z_draw_text_ex(spritefont, "Y=0",
                       SHIZVector2Make(y_bottom.x + spritefont.character.width / 2,
                                       y_bottom.y),
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                       SHIZSpriteFontSizeToFit,
                       y_color, attrs, axes_layer);
        
        z_draw_text_ex(spritefont, x_max,
                       SHIZVector2Make(x_right.x,
                                       x_right.y + (spritefont.character.height / 2) - 1),
                       SHIZSpriteFontAlignmentRight|SHIZSpriteFontAlignmentBottom,
                       SHIZSpriteFontSizeToFit,
                       x_color, attrs, axes_layer);
        z_draw_text_ex(spritefont, "X=0",
                       SHIZVector2Make(x_left.x,
                                       x_left.y - spritefont.character.height / 2),
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentTop,
                       SHIZSpriteFontSizeToFit,
                       x_color, attrs, axes_layer);
    }
}

void
z_debug__draw_sprite_gizmo(SHIZVector2 const location,
                           SHIZVector2 const anchor,
                           float const angle,
                           SHIZLayer const layer)
{
    if (angle > 0 || angle < 0) {
        float const radius = 6;
        unsigned int const segments = 8;
        
        SHIZVector2 const circle_center = SHIZVector2Make(location.x + anchor.x,
                                                          location.y + anchor.y);
        
        z_draw_circle_ex(circle_center,
                         SHIZColorWithAlpa(SHIZColorWhite, 0.1f),
                         SHIZDrawModeFill,
                         radius, segments,
                         SHIZLayeredBelow(layer));
        
        z_draw_arc_ex(circle_center,
                      SHIZColorWithAlpa(SHIZColorWhite, 0.6f),
                      SHIZDrawModeFill,
                      radius, segments,
                      angle,
                      layer);
    }
    
    float const anchor_size = 2;
    
    SHIZRect const anchor_rect =
    SHIZRectMake(location, SHIZSizeMake(anchor_size, anchor_size));
    
    z_draw_rect_ex(anchor_rect,
                   SHIZColorYellow,
                   SHIZDrawModeFill,
                   SHIZAnchorInverse(anchor),
                   angle,
                   SHIZLayeredAbove(layer));
}

void
z_debug__draw_sprite_shape(SHIZVector2 const origin,
                           SHIZSize const size,
                           SHIZColor const color,
                           SHIZVector2 const anchor,
                           float const angle,
                           SHIZLayer const layer)
{
    bool const previously_tracking_events = z_debug__is_events_enabled();
    
    z_debug__set_events_enabled(false);
    
    SHIZSize const padded_size = SHIZSizeMake(size.width + 1, size.height + 1);
    SHIZVector2 padded_origin = SHIZVector2Make(origin.x, origin.y);
    
    if (anchor.x > 0) {
        padded_origin.x += anchor.x;
    }
    
    if (anchor.y > 0) {
        padded_origin.y += anchor.y;
    }
    
    z_draw_rect_ex(SHIZRectMake(padded_origin, padded_size),
                   color, SHIZDrawModeOutline, anchor, angle,
                   SHIZLayeredAbove(layer));
    
    z_debug__draw_sprite_gizmo(origin, anchor, angle,
                               SHIZLayeredAbove(layer));
    
    z_debug__set_events_enabled(previously_tracking_events);
}

#endif
