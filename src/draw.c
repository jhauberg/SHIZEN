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

#include <SHIZEN/draw.h>
#include <SHIZEN/ztime.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "internal.h"
#include "sprite.h"
#include "spritefont.h"
#include "gfx.h"
#include "res.h"
#include "io.h"

static void _shiz_draw_path_3d(SHIZVector3 const points[],
                               unsigned int const count,
                               SHIZColor const color);

static void _shiz_draw_line_3d(SHIZVector3 const from,
                               SHIZVector3 const to,
                               SHIZColor const color);

static void _shiz_drawing_flush(void);

#ifdef SHIZ_DEBUG
static void _shiz_debug_build_stats(void);
static void _shiz_debug_draw_stats(void);
static void _shiz_debug_draw_events(void);
static void _shiz_debug_draw_viewport(void);
static void _shiz_debug_draw_sprite_gizmo(SHIZVector2 const location,
                                          SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer);
static void _shiz_debug_draw_sprite_shape(SHIZVector2 const origin,
                                          SHIZSize const size,
                                          SHIZColor const color,
                                          SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer);

static char _shiz_debug_stats_buffer[256] = { 0 };
#endif

extern SHIZGraphicsContext const _context;

void
shiz_drawing_begin(SHIZColor const background)
{
    shiz_sprite_reset();

    shiz_gfx_begin(background);

#ifdef SHIZ_DEBUG
    shiz_debug_reset_events();
#endif
}

void
shiz_drawing_end()
{
    _shiz_drawing_flush();

#ifdef SHIZ_DEBUG
    if (shiz_debug_is_enabled()) {
        bool const previously_drawing_shapes = shiz_debug_is_drawing_shapes();
        bool const previously_enabled_events = shiz_debug_is_events_enabled();

        shiz_debug_set_drawing_shapes(false);
        shiz_debug_set_events_enabled(false);

        _shiz_debug_draw_viewport();
        _shiz_debug_build_stats();
        _shiz_debug_draw_stats();

        if (shiz_debug_is_drawing_events()) {
            _shiz_debug_draw_events();
        }

        _shiz_drawing_flush();

        shiz_debug_set_drawing_shapes(previously_drawing_shapes);
        shiz_debug_set_events_enabled(previously_enabled_events);
    }
#endif

    shiz_gfx_end();

#ifdef SHIZ_DEBUG
    // flush out any encountered errors during this frame
    shiz_debug_process_errors();
#endif
    
    _shiz_present_frame();
}

void
shiz_draw_line(SHIZVector2 const from,
               SHIZVector2 const to,
               SHIZColor const color)
{
    shiz_draw_line_ex(from, to, color, SHIZLayerDefault);
}

void
shiz_draw_line_ex(SHIZVector2 const from,
                  SHIZVector2 const to,
                  SHIZColor const color,
                  SHIZLayer const layer)
{
    float const z = _shiz_layer_get_z(layer);
    
    _shiz_draw_line_3d(SHIZVector3Make(from.x, from.y, z),
                       SHIZVector3Make(to.x, to.y, z),
                       color);
}

void
shiz_draw_path(SHIZVector2 const points[],
               unsigned int const count,
               SHIZColor const color)
{
    shiz_draw_path_ex(points, count, color, SHIZLayerDefault);
}

void
shiz_draw_path_ex(SHIZVector2 const points[],
                  unsigned int const count,
                  SHIZColor const color,
                  SHIZLayer const layer) {
    SHIZVector3 points_3d[count];
    
    float const z = _shiz_layer_get_z(layer);
    
    for (unsigned int i = 0; i < count; i++) {
        points_3d[i].x = points[i].x;
        points_3d[i].y = points[i].y;
        points_3d[i].z = z;
    }
    
    _shiz_draw_path_3d(points_3d, count, color);
}

void
shiz_draw_point(SHIZVector2 const point, SHIZColor const color)
{
    shiz_draw_point_ex(point, color, SHIZLayerDefault);
}

void
shiz_draw_point_ex(SHIZVector2 const point, SHIZColor const color, SHIZLayer const layer)
{
    SHIZVertexPositionColor vertices[1] = {
        {
            .position = SHIZVector3Make(point.x, point.y, _shiz_layer_get_z(layer)),
            .color = color
        }
    };

    shiz_gfx_render(GL_POINTS, vertices, 1);
}

void
shiz_draw_rect(SHIZRect const rect, SHIZColor const color, SHIZDrawMode const mode)
{
    shiz_draw_rect_ex(rect, color, mode,
                      SHIZAnchorBottomLeft,
                      SHIZSpriteNoAngle,
                      SHIZLayerDefault);
}

void
shiz_draw_rect_ex(SHIZRect const rect,
                  SHIZColor const color,
                  SHIZDrawMode const mode,
                  SHIZVector2 const anchor,
                  float const angle,
                  SHIZLayer const layer)
{
    unsigned int const vertex_count = 4;

    SHIZVertexPositionColor vertices[vertex_count];

    for (unsigned int i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
    }

    float const z = _shiz_layer_get_z(layer);

    SHIZVector3 const origin = SHIZVector3Make(rect.origin.x, rect.origin.y, z);

    SHIZRect const anchored_rect = shiz_sprite_get_anchored_rect(rect.size, anchor);

    float const l = anchored_rect.origin.x;
    float const r = anchored_rect.origin.x + anchored_rect.size.width;
    float const b = anchored_rect.origin.y;
    float const t = anchored_rect.origin.y + anchored_rect.size.height;

    if (mode == SHIZDrawModeFill) {
        vertices[0].position = SHIZVector3Make(l, b, 0);
        vertices[1].position = SHIZVector3Make(l, t, 0);
        vertices[2].position = SHIZVector3Make(r, b, 0);
        vertices[3].position = SHIZVector3Make(r, t, 0);
    } else {
        vertices[0].position = SHIZVector3Make(l, b, 0);
        vertices[1].position = SHIZVector3Make(l, t, 0);
        // note that the order of the vertices differ from the filled shape
        vertices[2].position = SHIZVector3Make(r, t, 0);
        vertices[3].position = SHIZVector3Make(r, b, 0);
    }

    if (mode == SHIZDrawModeFill) {
        shiz_gfx_render_ex(GL_TRIANGLE_STRIP, vertices, vertex_count, origin, angle);
    } else {
        shiz_gfx_render_ex(GL_LINE_LOOP, vertices, vertex_count, origin, angle);
    }
}

void
shiz_draw_circle(SHIZVector2 const center,
                 SHIZColor const color,
                 SHIZDrawMode const mode,
                 float const radius,
                 unsigned int const segments)
{
    shiz_draw_circle_ex(center, color, mode, radius, segments,
                        SHIZLayerDefault);
}

void
shiz_draw_circle_ex(SHIZVector2 const center,
                    SHIZColor const color,
                    SHIZDrawMode const mode,
                    float const radius,
                    unsigned int const segments,
                    SHIZLayer const layer)
{
    unsigned int const vertex_count = mode == SHIZDrawModeFill ? (segments + 2) : segments;

    SHIZVertexPositionColor vertices[vertex_count];

    float const z = _shiz_layer_get_z(layer);
    float const step = 2.0f * M_PI / segments;

    SHIZVector3 const origin = SHIZVector3Make(center.x, center.y, z);

    unsigned int vertex_offset = 0;

    if (mode == SHIZDrawModeFill) {
        // start at the center
        vertices[0].color = color;
        vertices[0].position = SHIZVector3Zero;

        vertex_offset = 1;
    }

    for (unsigned int segment = 0; segment < segments; segment++) {
        unsigned int const vertex_index = vertex_offset + segment;

        float const angle = segment * step;

        float const x = radius * cosf(angle);
        float const y = radius * sinf(angle);

        vertices[vertex_index].color = color;
        vertices[vertex_index].position = SHIZVector3Make(x, y, 0);

        if (mode == SHIZDrawModeFill && segment == 0) {
            // connect the last vertex to the first shape vertex (i.e. not center in case of fill)
            unsigned int const last_vertex_index = vertex_offset + segments;

            vertices[last_vertex_index].color = color;
            vertices[last_vertex_index].position = vertices[vertex_index].position;
        }
    }

    if (mode == SHIZDrawModeFill) {
        shiz_gfx_render_ex(GL_TRIANGLE_FAN, vertices, vertex_count, origin,
                           SHIZSpriteNoAngle);
    } else {
        shiz_gfx_render_ex(GL_LINE_LOOP, vertices, vertex_count, origin,
                           SHIZSpriteNoAngle);
    }
}

void
shiz_draw_arc(SHIZVector2 const center,
              SHIZColor const color,
              SHIZDrawMode const mode,
              float const radius,
              unsigned int const segments,
              float const angle)
{
    shiz_draw_arc_ex(center, color, mode, radius, segments, angle,
                     SHIZLayerDefault);
}

void
shiz_draw_arc_ex(SHIZVector2 const center,
                 SHIZColor const color,
                 SHIZDrawMode const mode,
                 float const radius,
                 unsigned int const segments,
                 float const angle,
                 SHIZLayer const layer)
{
    unsigned int const vertex_count = segments + 2;

    SHIZVertexPositionColor vertices[vertex_count];

    float const target_angle = fmodf(angle, M_PI * 2.0f);

    float const z = _shiz_layer_get_z(layer);
    float const step = target_angle / segments;

    SHIZVector3 const origin = SHIZVector3Make(center.x, center.y, z);

    unsigned int vertex_offset = 1;

    vertices[0].color = color;
    vertices[0].position = SHIZVector3Zero;

    for (unsigned int segment = 0; segment < segments + 1; segment++) {
        unsigned int const vertex_index = vertex_offset + segment;

        float const step_angle = segment * step;

        float const x = radius * cosf(step_angle);
        float const y = radius * sinf(step_angle);

        vertices[vertex_index].color = color;
        vertices[vertex_index].position = SHIZVector3Make(x, y, 0);
    }

    if (mode == SHIZDrawModeFill) {
        shiz_gfx_render_ex(GL_TRIANGLE_FAN, vertices, vertex_count, origin, SHIZSpriteNoAngle);
    } else {
        shiz_gfx_render_ex(GL_LINE_LOOP, vertices, vertex_count, origin, SHIZSpriteNoAngle);
    }
}

SHIZSize
shiz_draw_sprite(SHIZSprite const sprite, SHIZVector2 const origin)
{
    return shiz_draw_sprite_ex(sprite, origin,
                               SHIZSpriteSizeIntrinsic,
                               SHIZAnchorCenter,
                               SHIZSpriteNoAngle,
                               SHIZSpriteNoTint,
                               SHIZSpriteNoRepeat,
                               SHIZSpriteNotOpaque,
                               SHIZLayerDefault);
}

SHIZSize
shiz_draw_sprite_ex(SHIZSprite const sprite,
                    SHIZVector2 const origin,
                    SHIZSpriteSize const size,
                    SHIZVector2 const anchor,
                    float const angle,
                    SHIZColor const tint,
                    bool const repeat,
                    bool const opaque,
                    SHIZLayer const layer)
{
    SHIZSize const sprite_size = shiz_sprite_draw(sprite, origin, size,
                                                  anchor, angle, tint,
                                                  repeat, opaque,
                                                  layer);

#ifdef SHIZ_DEBUG
    if (shiz_debug_is_enabled()) {
        if (shiz_debug_is_drawing_shapes() &&
            (sprite_size.width > 0 && sprite_size.height > 0)) {
            _shiz_debug_draw_sprite_shape(origin, sprite_size, SHIZColorRed,
                                          anchor, angle, layer);
        }

        shiz_debug_add_event_resource(shiz_res_get_image(sprite.resource_id).filename,
                                      SHIZVector3Make(origin.x, origin.y, 0));
    }
#endif

    return sprite_size;
}

SHIZSize
shiz_measure_sprite_text(SHIZSpriteFont const font,
                         char const * const text,
                         SHIZSize const bounds,
                         SHIZSpriteFontAttributes const attributes)
{
    SHIZSpriteFontMeasurement const measurement = shiz_sprite_measure_text(font, text,
                                                                           bounds, attributes);

    return measurement.size;
}

SHIZSize
shiz_draw_sprite_text(SHIZSpriteFont const font,
                      char const * const text,
                      SHIZVector2 const origin,
                      SHIZSpriteFontAlignment const alignment)
{
    return shiz_draw_sprite_text_ex(font, text, origin, alignment,
                                    SHIZSpriteFontSizeToFit,
                                    SHIZSpriteNoTint,
                                    SHIZSpriteFontAttributesDefault,
                                    SHIZLayerDefault);
}

SHIZSize
shiz_draw_sprite_text_ex(SHIZSpriteFont const font,
                         const char * const text,
                         SHIZVector2 const origin,
                         SHIZSpriteFontAlignment const alignment,
                         SHIZSize const bounds,
                         SHIZColor const tint,
                         SHIZSpriteFontAttributes const attributes,
                         SHIZLayer const layer)
{
    return shiz_draw_sprite_text_ex_colored(font, text,
                                            origin, alignment, bounds,
                                            tint, attributes, layer, NULL, 0);
}

SHIZSize
shiz_draw_sprite_text_ex_colored(SHIZSpriteFont const font,
                                 char const * const text,
                                 SHIZVector2 const origin,
                                 SHIZSpriteFontAlignment const alignment,
                                 SHIZSize const bounds,
                                 SHIZColor const tint,
                                 SHIZSpriteFontAttributes const attributes,
                                 SHIZLayer const layer,
                                 SHIZColor const * const highlight_colors,
                                 unsigned int const highlight_color_count)
{
    SHIZSize const text_size = shiz_sprite_draw_text(font, text,
                                                     origin, alignment, bounds,
                                                     tint, attributes, layer,
                                                     highlight_colors, highlight_color_count);

#ifdef SHIZ_DEBUG
    if (shiz_debug_is_enabled()) {
        if (shiz_debug_is_drawing_shapes() &&
            (text_size.width > 0 && text_size.height > 0)) {
            SHIZVector2 anchor = SHIZAnchorCenter;

            if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
                if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
                    anchor = SHIZAnchorTopLeft;
                } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                    anchor = SHIZAnchorTop;
                } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                    anchor = SHIZAnchorTopRight;
                }
            } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
                if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
                    anchor = SHIZAnchorLeft;
                } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                    anchor = SHIZAnchorCenter;
                } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                    anchor = SHIZAnchorRight;
                }
            } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
                if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
                    anchor = SHIZAnchorBottomLeft;
                } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                    anchor = SHIZAnchorBottom;
                } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                    anchor = SHIZAnchorBottomRight;
                }
            }

            // draw final size
            _shiz_debug_draw_sprite_shape(origin, text_size, SHIZColorRed, anchor,
                                          SHIZSpriteNoAngle,
                                          layer);

            if (bounds.width > 0 && bounds.height > 0) {
                // draw bounds
                _shiz_debug_draw_sprite_shape(origin, bounds, SHIZColorYellow, anchor,
                                              SHIZSpriteNoAngle,
                                              layer);
            }
        }

        shiz_debug_add_event_resource(shiz_res_get_image(font.sprite.resource_id).filename,
                                      SHIZVector3Make(origin.x, origin.y, 0));
    }
#endif

    return text_size;
}

static void
_shiz_draw_path_3d(SHIZVector3 const points[], unsigned int const count, SHIZColor const color)
{
    SHIZVertexPositionColor vertices[count];
    
    for (unsigned int i = 0; i < count; i++) {
        vertices[i].position = points[i];
        vertices[i].color = color;
    }
    
    shiz_gfx_render(GL_LINE_STRIP, vertices, count);
}

static void
_shiz_draw_line_3d(SHIZVector3 const from, SHIZVector3 const to, SHIZColor const color)
{
    SHIZVector3 points[] = {
        from, to
    };
    
    _shiz_draw_path_3d(points, 2, color);
}

static void
_shiz_drawing_flush()
{
    shiz_sprite_flush();
    shiz_gfx_flush();
}

#ifdef SHIZ_DEBUG
static void
_shiz_debug_build_stats()
{
    SHIZViewport const viewport = shiz_get_viewport();
    
    char const sprite_count_tint_specifier =
        shiz_debug_get_sprite_count() > SHIZSpriteInternalMax ?
            '\3' : '\2';
    
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
    
    SHIZDebugFrameStats const frame_stats = shiz_debug_get_frame_stats();
    
    bool const is_vsync_enabled = _context.swap_interval > 0;
    
    if (shiz_debug_is_expanded()) {
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
                sprite_count_tint_specifier, shiz_debug_get_sprite_count(), SHIZSpriteInternalMax,
                frame_stats.draw_count,
                shiz_get_time_lag() * 1000,
                shiz_get_tick_rate() * 1000,
                shiz_get_time_scale());
    } else {
        sprintf(_shiz_debug_stats_buffer,
                "\2%d fps%s\1",
                frame_stats.frames_per_second,
                is_vsync_enabled ? " (V)" : "");
    }
}

static void
_shiz_debug_draw_events()
{
    unsigned int const line_margin = 8;
    unsigned int const lane_margin = 4;
    unsigned int const lane_size = 8;

    unsigned int draw_events = 0;

    SHIZLayer const layer = SHIZLayerTop;

    SHIZSize const bounds = _context.native_size;

    for (unsigned int i = 0; i < shiz_debug_get_event_count(); i++) {
        SHIZDebugEvent const event = shiz_debug_get_event(i);

        float const lane_offset = lane_margin + (lane_size * event.lane) + (lane_margin * event.lane);

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

        _shiz_str_to_upper(event_buffer);

        shiz_draw_line_ex(SHIZVector2Make(from.x, from.y - line_margin - lane_margin), to, tint, layer);

        SHIZSize const event_text_size =
        shiz_draw_sprite_text_ex(shiz_debug_get_font(),
                                 event_buffer,
                                 from,
                                 SHIZSpriteFontAlignmentCenter | SHIZSpriteFontAlignmentTop,
                                 SHIZSpriteFontSizeToFit, tint,
                                 SHIZSpriteFontAttributesWithScale(1), layer);

        shiz_draw_rect_ex(SHIZRectMake(from, event_text_size),
                          SHIZColorBlack, SHIZDrawModeFill, SHIZAnchorTop, SHIZSpriteNoAngle,
                          SHIZLayeredBelow(layer));
    }
}

static void
_shiz_debug_draw_stats()
{
    SHIZLayer const layer = SHIZLayeredBelow(SHIZLayerTop);

    SHIZSpriteFont const spritefont = shiz_debug_get_font();

    unsigned int const margin = 8;

    SHIZColor highlight_colors[] = {
        SHIZColorFromHex(0xefec0d), // yellow
        SHIZColorFromHex(0xe5152d), // red
        SHIZColorFromHex(0x36cd33), // green
        SHIZColorFromHex(0x20b1fc)  // blue
    };

    SHIZSize const bounds = _context.native_size;

    SHIZVector2 stats_text_origin =
    SHIZVector2Make(bounds.width - margin,
                    bounds.height - margin);

    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault; {
        attrs.character_spread = SHIZSpriteFontSpreadTight;
    }
    
    shiz_draw_sprite_text_ex_colored(spritefont,
                                     _shiz_debug_stats_buffer,
                                     stats_text_origin,
                                     SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentRight,
                                     SHIZSpriteFontSizeToFit, SHIZSpriteNoTint,
                                     attrs, layer,
                                     highlight_colors, 4);

    char version_buffer[128] = { 0 };

    sprintf(version_buffer, "SHIZEN %d.%d.%d / %s (%s)",
            SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
            SHIZEN_VERSION_NAME, __DATE__);

    shiz_draw_sprite_text_ex(spritefont,
                             version_buffer,
                             SHIZVector2Make(bounds.width - margin / 2, margin / 2),
                             SHIZSpriteFontAlignmentBottom | SHIZSpriteFontAlignmentRight,
                             SHIZSpriteFontSizeToFit, SHIZSpriteTintDefaultWithAlpa(0.15f),
                             attrs, layer);
}

static void
_shiz_debug_draw_viewport()
{
    SHIZLayer const bounds_layer = SHIZLayerTop;
    
    SHIZViewport const viewport = shiz_get_viewport();
    
    SHIZVector2 const center = SHIZVector2Make(viewport.resolution.width / 2,
                                               viewport.resolution.height / 2);

    SHIZColor const bounds_color = SHIZColorWithAlpa(SHIZColorYellow, 0.8);

    SHIZColor const x_color = SHIZColorWithAlpa(SHIZColorFromHex(0x419fff), 0.6);
    SHIZColor const y_color = SHIZColorWithAlpa(SHIZColorGreen, 0.6);
    
    SHIZRect const viewport_shape = SHIZRectMake(center,
                                                 SHIZSizeMake(viewport.resolution.width - 1,
                                                              viewport.resolution.height - 1));
    
    // bounds
    shiz_draw_rect_ex(viewport_shape, bounds_color,
                      SHIZDrawModeOutline, SHIZAnchorCenter, 0, bounds_layer);
    
    // axes
    SHIZLayer const axes_layer = SHIZLayeredBelow(SHIZLayerTop);

    float const padding = 2;
    float const text_padding = 2;

    SHIZVector2 const y_bottom = SHIZVector2Make(center.x, padding + text_padding);
    SHIZVector2 const y_top = SHIZVector2Make(center.x, viewport.resolution.height - (padding + text_padding));

    SHIZVector2 const x_left = SHIZVector2Make(padding + text_padding, center.y);
    SHIZVector2 const x_right = SHIZVector2Make(viewport.resolution.width - (padding + text_padding), center.y);

    shiz_draw_line_ex(y_top, y_bottom,
                      y_color, axes_layer);
    shiz_draw_line_ex(x_left, x_right,
                      x_color, axes_layer);

    SHIZSpriteFont const spritefont = shiz_debug_get_font();
    
    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault; {
        attrs.character_spread = SHIZSpriteFontSpreadTight;
    }
    
    SHIZColor point_color = bounds_color;
    
    if (shiz_debug_is_expanded()) {
        shiz_draw_sprite_text_ex(spritefont, "0,0",
                                 SHIZVector2Make(spritefont.character.width / 2,
                                                 (spritefont.character.height / 2) - 1),
                                 SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                                 SHIZSpriteFontSizeToFit, point_color, attrs, axes_layer);
        
        char center_text[32] = { 0 };
        
        sprintf(center_text, "%.0f,%.0f", center.x, center.y);
        
        shiz_draw_sprite_text_ex(spritefont, center_text,
                                 SHIZVector2Make(center.x + spritefont.character.width / 2,
                                                 center.y + (spritefont.character.height / 2) - 1),
                                 SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                                 SHIZSpriteFontSizeToFit, point_color, attrs, axes_layer);
        
        char y_max[32] = { 0 };
        char x_max[32] = { 0 };

        sprintf(y_max, "Y=%.0f", viewport.resolution.height);
        sprintf(x_max, "X=%.0f", viewport.resolution.width);

        shiz_draw_sprite_text_ex(spritefont, y_max,
                                 SHIZVector2Make(y_top.x - spritefont.character.width / 2,
                                                 y_top.y),
                                 SHIZSpriteFontAlignmentRight|SHIZSpriteFontAlignmentTop,
                                 SHIZSpriteFontSizeToFit, y_color, attrs, axes_layer);
        shiz_draw_sprite_text_ex(spritefont, "Y=0",
                                 SHIZVector2Make(y_bottom.x + spritefont.character.width / 2,
                                                 y_bottom.y),
                                 SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentBottom,
                                 SHIZSpriteFontSizeToFit, y_color, attrs, axes_layer);

        shiz_draw_sprite_text_ex(spritefont, x_max,
                                 SHIZVector2Make(x_right.x,
                                                 x_right.y + (spritefont.character.height / 2) - 1),
                                 SHIZSpriteFontAlignmentRight|SHIZSpriteFontAlignmentBottom,
                                 SHIZSpriteFontSizeToFit, x_color, attrs, axes_layer);
        shiz_draw_sprite_text_ex(spritefont, "X=0",
                                 SHIZVector2Make(x_left.x,
                                                 x_left.y - spritefont.character.height / 2),
                                 SHIZSpriteFontAlignmentLeft|SHIZSpriteFontAlignmentTop,
                                 SHIZSpriteFontSizeToFit, x_color, attrs, axes_layer);
    }
}

static void
_shiz_debug_draw_sprite_gizmo(SHIZVector2 const location,
                              SHIZVector2 const anchor,
                              float const angle,
                              SHIZLayer const layer)
{
    if (angle > 0 || angle < 0) {
        float const radius = 6;
        unsigned int const segments = 8;

        SHIZVector2 const circle_center = SHIZVector2Make(location.x + anchor.x,
                                                          location.y + anchor.y);

        shiz_draw_circle_ex(circle_center, SHIZColorWithAlpa(SHIZColorWhite, 0.1f), SHIZDrawModeFill,
                            radius, segments, SHIZLayeredBelow(layer));

        shiz_draw_arc_ex(circle_center, SHIZColorWithAlpa(SHIZColorWhite, 0.6f), SHIZDrawModeFill,
                         radius, segments, angle, layer);
    }

    float const anchor_size = 2;

    SHIZRect const anchor_rect = SHIZRectMake(location, SHIZSizeMake(anchor_size,
                                                                     anchor_size));

    shiz_draw_rect_ex(anchor_rect, SHIZColorYellow, SHIZDrawModeFill,
                      SHIZAnchorInverse(anchor), angle,
                      SHIZLayeredAbove(layer));
}

static void
_shiz_debug_draw_sprite_shape(SHIZVector2 const origin,
                              SHIZSize const size,
                              SHIZColor const color,
                              SHIZVector2 const anchor,
                              float const angle,
                              SHIZLayer const layer)
{
    bool const previously_tracking_events = shiz_debug_is_events_enabled();

    shiz_debug_set_events_enabled(false);

    SHIZSize const padded_size = SHIZSizeMake(size.width + 1, size.height + 1);
    SHIZVector2 padded_origin = SHIZVector2Make(origin.x, origin.y);

    if (anchor.x > 0) {
        padded_origin.x += anchor.x;
    }

    if (anchor.y > 0) {
        padded_origin.y += anchor.y;
    }

    shiz_draw_rect_ex(SHIZRectMake(padded_origin, padded_size),
                      color, SHIZDrawModeOutline, anchor, angle, SHIZLayeredAbove(layer));

    _shiz_debug_draw_sprite_gizmo(origin, anchor, angle, SHIZLayeredAbove(layer));

    shiz_debug_set_events_enabled(previously_tracking_events);
}
#endif
