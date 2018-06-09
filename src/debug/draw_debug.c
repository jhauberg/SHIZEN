#ifdef SHIZ_DEBUG

#include <stdio.h> // sprintf

#include <SHIZEN/ztime.h>
#include <SHIZEN/zdraw.h>

#include "debug.h"
#include "profiler.h"

#include "../internal.h"
#include "../sprite.h"
#include "../spritefont.h"
#include "../viewport.h"
#include "../res.h"

static
void
z_debug__draw_gizmo(SHIZVector2 location,
                    SHIZVector2 anchor,
                    float angle,
                    SHIZLayer layer);

static
void
z_debug__draw_rect_bounds_ex(SHIZRect rect,
                             SHIZColor color,
                             SHIZVector2 anchor,
                             float angle,
                             SHIZLayer layer,
                             bool draw_gizmo);

extern SHIZGraphicsContext const _graphics_context;

static char _stats_buffer[256] = { 0 };

void
z_debug__build_stats()
{
    SHIZViewport const viewport = z_viewport__get();
    
    uint32_t const sprite_count = z_debug__get_sprite_count();
    
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
    
    SHIZProfilerStats const frame_stats = z_profiler__get_stats();
    
    bool const is_vsync_enabled = _graphics_context.swap_interval > 0;
    
    if (z_debug__is_expanded()) {
        sprintf(_stats_buffer,
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
                z_time_get_tick_rate() * 1000,
                z_time_get_scale());
    } else {
        sprintf(_stats_buffer,
                "\2%d fps%s\1",
                frame_stats.frames_per_second,
                is_vsync_enabled ? " (V)" : "");
    }
}

void
z_debug__draw_events()
{
    float const line_margin = 8;
    float const lane_margin = 4;
    float const lane_size = 8;
    
    uint16_t draw_events = 0;
    
    SHIZLayer const layer = SHIZLayerTop;
    
    SHIZSize const bounds = _graphics_context.native_size;
    
    for (uint16_t i = 0; i < z_debug__get_event_count(); i++) {
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
                           SHIZSpriteFontSizeToFit,
                           SHIZSpriteFontAttributesWithScale(1),
                           SHIZSpriteFontAlignmentCenter|SHIZSpriteFontAlignmentTop,
                           tint,
                           layer);
        
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
    
    float const margin = 8;
    
    SHIZColor const highlight_colors[] = {
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
        attrs.colors = highlight_colors;
        attrs.colors_count = 4;
    }
    
    SHIZSize const stats_size =
        z_measure_text_attributed(spritefont,
                                  _stats_buffer,
                                  SHIZSpriteFontSizeToFit,
                                  attrs);
    
    z_draw_rect_ex(SHIZRectMake(stats_text_origin, stats_size),
                   SHIZColorWithAlpa(SHIZColorBlack, 0.6f),
                   SHIZDrawModeFill,
                   SHIZAnchorTopRight,
                   SHIZSpriteNoAngle,
                   SHIZLayeredBelow(layer));
    
    z_draw_text_attributed(spritefont,
                           _stats_buffer,
                           stats_text_origin,
                           SHIZSpriteFontSizeToFit,
                           attrs,
                           SHIZSpriteFontParametersMake(SHIZSpriteFontAlignmentTop|SHIZSpriteFontAlignmentRight,
                                                        layer,
                                                        SHIZSpriteNoTint));
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
        SHIZRectMake(SHIZVector2Zero, SHIZSizeMake((viewport.resolution.width), (viewport.resolution.height)));
    
    // bounds
    z_draw_rect_ex(viewport_shape, bounds_color,
                   SHIZDrawModeOutline, SHIZAnchorBottomLeft, 0, bounds_layer);
    
    if (!z_debug__is_drawing_axes()) {
        return;
    }
    
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
    
    if (z_debug__is_expanded()) {
        SHIZSpriteFont const spritefont = z_debug__get_font();
        
        SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault; {
            attrs.character_spread = SHIZSpriteFontSpreadTight;
        }
        
        SHIZColor point_color = bounds_color;
        
        z_draw_text_ex(spritefont, "0,0",
                       SHIZVector2Make(spritefont.character.width / 2,
                                       (spritefont.character.height / 2) - 1),
                       SHIZSpriteFontSizeToFit,
                       attrs,
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                       point_color, axes_layer);
        
        char center_text[32] = { 0 };
        
        sprintf(center_text, "%.0f,%.0f", center.x, center.y);
        
        z_draw_text_ex(spritefont, center_text,
                       SHIZVector2Make(center.x + spritefont.character.width / 2,
                                       center.y + (spritefont.character.height / 2) - 1),
                       SHIZSpriteFontSizeToFit,
                       attrs,
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                       point_color, axes_layer);
        
        char y_max[32] = { 0 };
        char x_max[32] = { 0 };
        
        sprintf(y_max, "Y=%.0f", viewport.resolution.height);
        sprintf(x_max, "X=%.0f", viewport.resolution.width);
        
        z_draw_text_ex(spritefont, y_max,
                       SHIZVector2Make(y_top.x - spritefont.character.width / 2,
                                       y_top.y),
                       SHIZSpriteFontSizeToFit,
                       attrs,
                       SHIZSpriteFontAlignmentRight|SHIZSpriteFontAlignmentTop,
                       y_color, axes_layer);
        z_draw_text_ex(spritefont, "Y=0",
                       SHIZVector2Make(y_bottom.x + spritefont.character.width / 2,
                                       y_bottom.y),
                       SHIZSpriteFontSizeToFit,
                       attrs,
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                       y_color, axes_layer);
        
        z_draw_text_ex(spritefont, x_max,
                       SHIZVector2Make(x_right.x,
                                       x_right.y + (spritefont.character.height / 2) - 1),
                       SHIZSpriteFontSizeToFit,
                       attrs,
                       SHIZSpriteFontAlignmentRight|SHIZSpriteFontAlignmentBottom,
                       x_color, axes_layer);
        z_draw_text_ex(spritefont, "X=0",
                       SHIZVector2Make(x_left.x,
                                       x_left.y - spritefont.character.height / 2),
                       SHIZSpriteFontSizeToFit,
                       attrs,
                       SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentTop,
                       x_color, axes_layer);
    }
}

void
z_debug__draw_rect_bounds(SHIZRect const rect,
                          SHIZColor const color,
                          SHIZVector2 const anchor,
                          float const angle,
                          SHIZLayer const layer)
{
    z_debug__draw_rect_bounds_ex(rect, color, anchor, angle, layer, true);
}

void
z_debug__draw_sprite_bounds(SHIZVector2 const origin,
                            SHIZSize const size,
                            SHIZColor const color,
                            SHIZVector2 const anchor,
                            float const angle,
                            SHIZLayer const layer)
{
    z_debug__draw_rect_bounds(SHIZRectMake(origin, size),
                              color,
                              anchor,
                              angle,
                              layer);
}

void
z_debug__draw_circle_bounds(SHIZVector2 const origin,
                            SHIZColor const color,
                            float const radius,
                            SHIZVector2 const scale,
                            SHIZLayer const layer)
{
    SHIZRect const rect =
        SHIZRectMake(origin, SHIZSizeMake((radius * scale.x) * 2,
                                          (radius * scale.y) * 2));
    
    z_debug__draw_rect_bounds_ex(rect, color,
                                 SHIZAnchorCenter, 0, layer, false);
}

void
z_debug__draw_points_bounds(SHIZVector2 const points[], uint16_t const count,
                            SHIZColor const color,
                            float const angle,
                            SHIZLayer const layer)
{
    SHIZRect rect = SHIZRectFromPoints(points, count);
    
    rect.origin = SHIZVector2Make(rect.origin.x + rect.size.width / 2,
                                  rect.origin.y + rect.size.height / 2);
    
    bool const show_gizmo = angle > 0 || angle < 0;
    
    z_debug__draw_rect_bounds_ex(rect, color,
                                 SHIZAnchorCenter,
                                 angle, layer, show_gizmo);
}

static
void
z_debug__draw_gizmo(SHIZVector2 const location,
                    SHIZVector2 const anchor,
                    float const angle,
                    SHIZLayer const layer)
{
    if (angle > 0 || angle < 0) {
        uint8_t const segments = 10;
        float const radius = 6;
        
        z_draw_circle_ex(location,
                         SHIZColorWithAlpa(SHIZColorWhite, 0.1f),
                         SHIZDrawModeFill,
                         radius, segments,
                         SHIZVector2One,
                         SHIZLayeredBelow(layer));
        
        z_draw_arc_ex(location,
                      SHIZColorWithAlpa(SHIZColorWhite, 0.6f),
                      SHIZDrawModeFill,
                      radius, segments,
                      angle,
                      layer);
    }
    
    float const anchor_size = 2;
    
    SHIZRect const anchor_rect =
        SHIZRectMake(location, SHIZSizeMake(anchor_size, anchor_size));
    
    bool const anchor_outside = false;
    
    z_draw_rect_ex(anchor_rect,
                   SHIZColorYellow,
                   SHIZDrawModeFill,
                   anchor_outside ? SHIZAnchorInverse(anchor) : anchor,
                   0,
                   SHIZLayeredAbove(layer));
}

static
void
z_debug__draw_rect_bounds_ex(SHIZRect const rect,
                             SHIZColor const color,
                             SHIZVector2 const anchor,
                             float const angle,
                             SHIZLayer const layer,
                             bool const draw_gizmo)
{
    bool const previously_tracking_events = z_debug__is_events_enabled();
    
    z_debug__set_events_enabled(false);
    
    z_profiler__set_is_profiling(false);
    
    // enable outside bounds to avoid the frame overlapping sprite/primitive
    // pixels, but it won't work well with rotated elements
    bool const outside_bounds = false;
    
    SHIZRect bounds_rect = rect;
    
    if (outside_bounds) {
        bounds_rect.size = SHIZSizeMake(bounds_rect.size.width + 2,
                                        bounds_rect.size.height + 2);
        
        bounds_rect.origin =
            SHIZVector2Make(bounds_rect.origin.x + (1 * anchor.x),
                            bounds_rect.origin.y + (1 * anchor.y));
    }
    
    bool const previously_drawing_shapes = z_debug__is_drawing_shapes();
    
    // disable shape drawing while drawing this shape to avoid
    // drawing the shape of a shape of a shape...
    z_debug__set_drawing_shapes(false);
    
    z_draw_rect_ex(bounds_rect,
                   SHIZColorWithAlpa(color, 0.6f), SHIZDrawModeOutline, anchor, angle,
                   SHIZLayeredAbove(layer));
    
    if (draw_gizmo) {
        z_debug__draw_gizmo(rect.origin, anchor, angle, SHIZLayeredAbove(layer));
    }
    
    z_profiler__set_is_profiling(true);
    
    z_debug__set_drawing_shapes(previously_drawing_shapes);
    z_debug__set_events_enabled(previously_tracking_events);
}

#endif
