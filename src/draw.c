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
#include <SHIZEN/time.h>

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

#ifdef SHIZ_DEBUG
static void _shiz_debug_build_stats(void);
static void _shiz_debug_draw_stats(void);
static void _shiz_debug_draw_events(void);
static void _shiz_debug_draw_viewport(void);
static void _shiz_debug_draw_sprite_gizmo(SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer);
static void _shiz_debug_draw_sprite_shape(SHIZVector2 const origin,
                                          SHIZSize const size,
                                          SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer);

static char _shiz_debug_stats_buffer[256];
#endif

void
shiz_drawing_begin()
{
#ifdef SHIZ_DEBUG
    shiz_debug_reset_events();
#endif

    shiz_sprite_reset();

    shiz_gfx_begin();
}

void
shiz_drawing_end()
{
    shiz_sprite_flush();
    shiz_gfx_flush();

#ifdef SHIZ_DEBUG
    // draw debug stuff lastly, but before ending the frame- it becomes part of the post-process
    // to ensure consistency with the rendered shapes/sprites
    if (shiz_debug_is_enabled()) {
        bool const previously_drawing_shapes = shiz_debug_is_drawing_shapes();
        
        // disable shapes during drawing of debug things
        shiz_debug_set_drawing_shapes(false);
        // disable event tracking as well (this is enabled again at the beginning of next frame)
        shiz_debug_set_events_enabled(false);

        _shiz_debug_draw_viewport();
        _shiz_debug_build_stats();
        _shiz_debug_draw_stats();

        if (shiz_debug_is_drawing_events()) {
            _shiz_debug_draw_events();
        }

        shiz_sprite_flush();
        shiz_gfx_flush();

        // reset back to previous settings
        shiz_debug_set_events_enabled(true);
        shiz_debug_set_drawing_shapes(previously_drawing_shapes);
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
shiz_draw_line(SHIZVector2 const from, SHIZVector2 const to, SHIZColor const color)
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
                      SHIZSpriteNoAngle, SHIZLayerDefault);
}

void
shiz_draw_rect_ex(SHIZRect const rect,
                  SHIZColor const color,
                  SHIZDrawMode const mode,
                  SHIZVector2 const anchor,
                  float const angle,
                  SHIZLayer const layer)
{
    unsigned int const vertex_count = mode == SHIZDrawModeFill ? 4 : 5; // only drawing the shape requires an additional vertex

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
        // the additional vertex connects to the beginning, to complete the shape
        vertices[4].position = vertices[0].position;
    }

    if (mode == SHIZDrawModeFill) {
        shiz_gfx_render_ex(GL_TRIANGLE_STRIP, vertices, vertex_count, origin, angle);
    } else {
        shiz_gfx_render_ex(GL_LINE_STRIP, vertices, vertex_count, origin, angle);
    }
}

void
shiz_draw_circle(SHIZVector2 const center,
                 float const radius,
                 unsigned int const segments,
                 SHIZDrawMode const mode,
                 SHIZColor const color)
{
    shiz_draw_circle_ex(center, radius, segments, mode, color, SHIZLayerDefault);
}

void
shiz_draw_circle_ex(SHIZVector2 const center,
                    float const radius,
                    unsigned int const segments,
                    SHIZDrawMode const mode,
                    SHIZColor const color,
                    SHIZLayer const layer)
{
    unsigned int const vertex_count = mode == SHIZDrawModeFill ? (segments + 2) : (segments + 1);

    SHIZVertexPositionColor vertices[vertex_count];

    float const z = _shiz_layer_get_z(layer);
    float const step = 2.0f * M_PI / segments;

    SHIZVector3 const origin = SHIZVector3Make(center.x, center.y, z);

    unsigned int const vertex_offset = mode == SHIZDrawModeFill ? 1 : 0;

    if (mode == SHIZDrawModeFill) {
        // start at the center
        vertices[0].color = color;
        vertices[0].position = SHIZVector3Zero;
    }

    for (unsigned int segment = 0; segment < segments; segment++) {
        unsigned int const vertex_index = vertex_offset + segment;

        float const angle = segment * step;

        float const x = radius * cosf(angle);
        float const y = radius * sinf(angle);

        vertices[vertex_index].color = color;
        vertices[vertex_index].position = SHIZVector3Make(x, y, 0);

        if (segment == 0) {
            // connect the last vertex to the first shape vertex (i.e. not center in case of fill)
            unsigned int const last_vertex_index = vertex_offset + segments;

            vertices[last_vertex_index].color = color;
            vertices[last_vertex_index].position = vertices[vertex_index].position;
        }
    }

    if (mode == SHIZDrawModeFill) {
        shiz_gfx_render_ex(GL_TRIANGLE_FAN, vertices, vertex_count, origin, SHIZSpriteNoAngle);
    } else {
        shiz_gfx_render_ex(GL_LINE_STRIP, vertices, vertex_count, origin, SHIZSpriteNoAngle);
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
                               false,
                               SHIZLayerDefault);
}

SHIZSize
shiz_draw_sprite_ex(SHIZSprite const sprite,
                    SHIZVector2 const origin,
                    SHIZSize const size,
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
            _shiz_debug_draw_sprite_shape(origin, sprite_size, anchor, angle, layer);
        }

        shiz_debug_add_event_resource(shiz_res_get_image(sprite.resource_id).filename,
                                      SHIZVector3Make(origin.x, origin.y, 0));
    }
#endif

    return sprite_size;
}

SHIZSize
shiz_measure_sprite_text(SHIZSpriteFont const font,
                         const char * const text,
                         SHIZSize const bounds,
                         SHIZSpriteFontAttributes const attributes)
{
    SHIZSpriteFontMeasurement const measurement = shiz_sprite_measure_text(font, text,
                                                                           bounds, attributes);

    return measurement.size;
}

SHIZSize
shiz_draw_sprite_text(SHIZSpriteFont const font,
                      const char * const text,
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
                                 const char * const text,
                                 SHIZVector2 const origin,
                                 SHIZSpriteFontAlignment const alignment,
                                 SHIZSize const bounds,
                                 SHIZColor const tint,
                                 SHIZSpriteFontAttributes const attributes,
                                 SHIZLayer const layer,
                                 SHIZColor * const highlight_colors,
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

            _shiz_debug_draw_sprite_shape(origin, text_size, anchor,
                                          SHIZSpriteNoAngle,
                                          SHIZLayerDefault);
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

#ifdef SHIZ_DEBUG
static void
_shiz_debug_build_stats()
{
    SHIZViewport const viewport = shiz_gfx_get_viewport();
    
    char const sprite_count_tint_specifier =
        shiz_debug_get_sprite_count() > SHIZSpriteInternalMax ? '\3' : '\2';
    
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
    
    sprintf(_shiz_debug_stats_buffer,
            "%s\n\n"
            "\2%0.2fms/frame\1 (\4%0.2fms\1)\n"
            "\2%d fps\1 (\3%d\x19\1 \4%d\x12\1 \5%d\x18\1)\n\n"
            "%c%d/%d sprites/frame\1\n"
            "\2%d draws/frame\1\n\n"
            "\2%.1fx time\1",
            display_size_buffer,
            shiz_gfx_debug_get_frame_time(),
            shiz_gfx_debug_get_frame_time_avg(),
            shiz_gfx_debug_get_frames_per_second(),
            shiz_gfx_debug_get_frames_per_second_min(),
            shiz_gfx_debug_get_frames_per_second_avg(),
            shiz_gfx_debug_get_frames_per_second_max(),
            sprite_count_tint_specifier, shiz_debug_get_sprite_count(), SHIZSpriteInternalMax,
            shiz_gfx_debug_get_draw_count(),
            shiz_get_time_scale());
}

static void
_shiz_debug_draw_events()
{
    unsigned int const line_margin = 8;
    unsigned int const lane_margin = 4;
    unsigned int const lane_size = 8;

    unsigned int draw_events = 0;

    SHIZLayer const layer = SHIZLayerTop;

    SHIZSize const display_size = _shiz_get_operating_resolution();

    for (unsigned int i = 0; i < shiz_debug_get_event_count(); i++) {
        SHIZDebugEvent const event = shiz_debug_get_event(i);

        float const lane_offset = lane_margin + (lane_size * event.lane) + (lane_margin * event.lane);

        SHIZVector2 const from = SHIZVector2Make(event.origin.x,
                                                 display_size.height - lane_offset);
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

    unsigned int const margin = 8;

    SHIZColor highlight_colors[] = {
        SHIZColorFromHex(0xefec0d), // yellow
        SHIZColorFromHex(0xe5152d), // red
        SHIZColorFromHex(0x36cd33), // green
        SHIZColorFromHex(0x20b1fc)  // blue
    };

    SHIZSize const display_size = _shiz_get_operating_resolution();

    SHIZVector2 stats_text_origin =
    SHIZVector2Make(display_size.width - margin,
                    display_size.height - margin);

    SHIZSpriteFontAttributes attrs = SHIZSpriteFontAttributesDefault; {
        attrs.character_spread = SHIZSpriteFontSpreadTight;
    }
    
    shiz_draw_sprite_text_ex_colored(shiz_debug_get_font(),
                                     _shiz_debug_stats_buffer,
                                     stats_text_origin,
                                     SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentRight,
                                     SHIZSpriteFontSizeToFit, SHIZSpriteNoTint,
                                     attrs, layer,
                                     highlight_colors, 4);

    char version_buffer[128];

    sprintf(version_buffer, "SHIZEN %d.%d.%d / %s (%s)",
            SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
            SHIZEN_VERSION_NAME, __DATE__);
    
    shiz_draw_sprite_text_ex(shiz_debug_get_font(),
                             version_buffer,
                             SHIZVector2Make(margin / 2, margin / 2),
                             SHIZSpriteFontAlignmentBottom | SHIZSpriteFontAlignmentLeft,
                             SHIZSpriteFontSizeToFit, SHIZSpriteTintDefaultWithAlpa(0.25f),
                             attrs, layer);
}

static void
_shiz_debug_draw_viewport()
{
    SHIZLayer const layer = SHIZLayeredBelow(SHIZLayeredBelow(SHIZLayerTop));
    
    SHIZViewport const viewport = shiz_gfx_get_viewport();
    
    SHIZVector2 const center = SHIZVector2Make(viewport.resolution.width / 2,
                                               viewport.resolution.height / 2);
    
    SHIZColor color = SHIZColorWithAlpa(SHIZColorRed, 0.8);
    
    SHIZRect viewport_shape = SHIZRectMake(center, SHIZSizeMake(viewport.resolution.width - 1,
                                                                viewport.resolution.height - 1));
    
    // bounds
    shiz_draw_rect_ex(viewport_shape, color, SHIZDrawModeOutline, SHIZAnchorCenter, 0, layer);
    
    // center grid
    float const padding = 24;
    
    shiz_draw_line_ex(SHIZVector2Make(center.x, viewport.resolution.height - padding),
                      SHIZVector2Make(center.x, padding),
                      color, layer);
    shiz_draw_line_ex(SHIZVector2Make(padding, center.y),
                      SHIZVector2Make(viewport.resolution.width - padding, center.y),
                      color, layer);
}

static void
_shiz_debug_draw_sprite_gizmo(SHIZVector2 const anchor, float const angle, SHIZLayer const layer)
{
    float const axis_length = 5;
    
    SHIZLayer const layer_above = SHIZLayeredAbove(layer);
    
    float const z = _shiz_layer_get_z(layer_above);
    
    SHIZVector3 const origin = SHIZVector3Make(anchor.x, anchor.y, z);
    
    SHIZVertexPositionColor vertices[2];
    
    vertices[0].position = SHIZVector3Zero;
    
    // x axis
    vertices[1].position = SHIZVector3Make(axis_length, 0, 0);
    vertices[0].color = SHIZColorBlue;
    vertices[1].color = SHIZColorBlue;
    
    shiz_gfx_render_ex(GL_LINE_STRIP, vertices, 2, origin, angle);
    
    // y axis
    vertices[1].position = SHIZVector3Make(0, axis_length, 0);
    vertices[0].color = SHIZColorGreen;
    vertices[1].color = SHIZColorGreen;
    
    shiz_gfx_render_ex(GL_LINE_STRIP, vertices, 2, origin, angle);

    // anchor
    float const anchor_size = 2;
    
    shiz_draw_rect_ex(SHIZRectMake(anchor, SHIZSizeMake(anchor_size, anchor_size)),
                      SHIZColorRed, SHIZDrawModeFill, SHIZAnchorCenter, angle, layer_above);
}

static void
_shiz_debug_draw_sprite_shape(SHIZVector2 const origin,
                              SHIZSize const size,
                              SHIZVector2 const anchor,
                              float const angle,
                              SHIZLayer const layer)
{
    bool const previously_tracking_events = shiz_debug_is_events_enabled();

    shiz_debug_set_events_enabled(false);

    SHIZSize const padded_size = SHIZSizeMake(size.width + 1, size.height + 1);
    SHIZVector2 const padded_origin = SHIZVector2Make(origin.x - 0, origin.y - 0);
    
    SHIZLayer const layer_above = SHIZLayeredAbove(layer);
    
    shiz_draw_rect_ex(SHIZRectMake(padded_origin, padded_size),
                      SHIZColorRed, SHIZDrawModeOutline, anchor, angle, layer_above);

    _shiz_debug_draw_sprite_gizmo(origin, angle, layer_above);

    shiz_debug_set_events_enabled(previously_tracking_events);
}
#endif
