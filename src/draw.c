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

static void _shiz_draw_rect(SHIZRect const rect,
                            SHIZColor const color,
                            bool const fill,
                            SHIZVector2 const anchor,
                            float const angle,
                            SHIZLayer const layer);

static void _shiz_draw_path_3d(SHIZVector3 const points[], uint const count, SHIZColor const color);
static void _shiz_draw_line_3d(SHIZVector3 const from, SHIZVector3 const to, SHIZColor const color);

#ifdef SHIZ_DEBUG
static void _shiz_debug_event(SHIZDebugEvent const event);
static void _shiz_debug_process_errors(void);
static void _shiz_debug_build_stats(void);
static void _shiz_debug_draw_stats(void);
static void _shiz_debug_draw_events(void);
static void _shiz_debug_draw_sprite_gizmo(SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer);
static void _shiz_debug_draw_sprite_shape(SHIZVector2 const origin,
                                          SHIZSize const size,
                                          SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer);
static void _shiz_debug_event_sprite_resource(SHIZVector2 const origin,
                                              SHIZResourceImage const resource);

static char _shiz_debug_stats_buffer[256];
#endif

void shiz_drawing_begin() {
    shiz_gfx_clear();

#ifdef SHIZ_DEBUG
    shiz_gfx_debug_set_event_callback(_shiz_debug_event);

    shiz_debug_context.event_count = 0;
    shiz_debug_context.sprite_count = 0;
    shiz_debug_context.is_tracking_enabled = true;
#endif
    
    shiz_gfx_begin();
}

void shiz_drawing_end() {
    shiz_sprite_flush();

    shiz_gfx_end();

#ifdef SHIZ_DEBUG
    if (shiz_debug_context.is_enabled) {
        shiz_debug_context.is_tracking_enabled = false;

        bool const previously_drawing_shapes = shiz_debug_context.draw_sprite_shape;

        shiz_debug_context.draw_sprite_shape = false;

        _shiz_debug_build_stats();

        if (shiz_debug_context.draw_events) {
            _shiz_debug_draw_events();
        }

        _shiz_debug_draw_stats();

        shiz_sprite_flush();

        shiz_gfx_flush();

        shiz_debug_context.draw_sprite_shape = previously_drawing_shapes;
    }

    _shiz_debug_process_errors();
#endif

    glfwSwapBuffers(shiz_context.window);
    glfwPollEvents();
}

static void _shiz_draw_path_3d(SHIZVector3 const points[], uint const count, SHIZColor const color) {
    SHIZVertexPositionColor vertices[count];

    for (uint i = 0; i < count; i++) {
        vertices[i].position = points[i];
        vertices[i].color = color;
    }

    shiz_gfx_render(GL_LINE_STRIP, vertices, count);
}

static void _shiz_draw_line_3d(SHIZVector3 const from, SHIZVector3 const to, SHIZColor const color) {
    SHIZVector3 points[] = {
        from, to
    };

    _shiz_draw_path_3d(points, 2, color);
}

void shiz_draw_line(SHIZVector2 const from, SHIZVector2 const to, SHIZColor const color) {
    shiz_draw_line_ex(from, to, color, SHIZLayerDefault);
}

void shiz_draw_line_ex(SHIZVector2 const from, SHIZVector2 const to, SHIZColor const color, SHIZLayer const layer) {
    float const z = _shiz_layer_get_z(layer);
    
    _shiz_draw_line_3d(SHIZVector3Make(from.x, from.y, z),
                       SHIZVector3Make(to.x, to.y, z),
                       color);
}

void shiz_draw_path(SHIZVector2 const points[], uint const count, SHIZColor const color) {
    shiz_draw_path_ex(points, count, color, SHIZLayerDefault);
}

void shiz_draw_path_ex(SHIZVector2 const points[], uint const count, SHIZColor const color, SHIZLayer const layer) {
    SHIZVector3 points3[count];
    
    float const z = _shiz_layer_get_z(layer);
    
    for (uint i = 0; i < count; i++) {
        points3[i].x = points[i].x;
        points3[i].y = points[i].y;
        points3[i].z = z;
    }
    
    _shiz_draw_path_3d(points3, count, color);
}

static void _shiz_draw_rect(SHIZRect const rect,
                            SHIZColor const color,
                            bool const fill,
                            SHIZVector2 const anchor,
                            float const angle,
                            SHIZLayer const layer) {
    uint const vertex_count = fill ? 4 : 5; // only drawing the shape requires an additional vertex

    SHIZVertexPositionColor vertices[vertex_count];

    for (uint i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
    }

    SHIZVector3 const origin = SHIZVector3Make(rect.origin.x, rect.origin.y, 0);

    SHIZRect const anchored_rect = shiz_sprite_get_anchored_rect(rect.size, anchor);
    
    float const l = anchored_rect.origin.x;
    float const r = anchored_rect.origin.x + anchored_rect.size.width;
    float const b = anchored_rect.origin.y;
    float const t = anchored_rect.origin.y + anchored_rect.size.height;
    
    float const z = _shiz_layer_get_z(layer);
    
    if (!fill) {
        vertices[0].position = SHIZVector3Make(l, b, z);
        vertices[1].position = SHIZVector3Make(l, t, z);
        // note that the order of the vertices differ from the filled shape
        vertices[2].position = SHIZVector3Make(r, t, z);
        vertices[3].position = SHIZVector3Make(r, b, z);
        // the additional vertex connects to the beginning, to complete the shape
        vertices[4].position = vertices[0].position;

        shiz_gfx_render_ex(GL_LINE_STRIP, vertices, vertex_count, origin, angle);
    } else {
        vertices[0].position = SHIZVector3Make(l, b, z);
        vertices[1].position = SHIZVector3Make(l, t, z);
        vertices[2].position = SHIZVector3Make(r, b, z);
        vertices[3].position = SHIZVector3Make(r, t, z);

        shiz_gfx_render_ex(GL_TRIANGLE_STRIP, vertices, vertex_count, origin, angle);
    }
}

void shiz_draw_rect(SHIZRect const rect, SHIZColor const color) {
    _shiz_draw_rect(rect, color, true,
                    SHIZSpriteAnchorBottomLeft,
                    SHIZSpriteNoAngle, SHIZLayerDefault);
}

void shiz_draw_rect_ex(SHIZRect const rect,
                       SHIZColor const color,
                       SHIZVector2 const anchor,
                       float const angle,
                       SHIZLayer const layer) {
    _shiz_draw_rect(rect, color, true, anchor, angle, layer);
}

void shiz_draw_rect_shape(SHIZRect const rect, SHIZColor const color) {
    _shiz_draw_rect(rect, color, false,
                    SHIZSpriteAnchorBottomLeft,
                    SHIZSpriteNoAngle, SHIZLayerDefault);
}

void shiz_draw_rect_shape_ex(SHIZRect const rect,
                             SHIZColor const color,
                             SHIZVector2 const anchor,
                             float const angle,
                             SHIZLayer const layer) {
    _shiz_draw_rect(rect, color, false, anchor, angle, layer);
}

SHIZSize shiz_draw_sprite(SHIZSprite const sprite, SHIZVector2 const origin) {
    return shiz_draw_sprite_ex(sprite, origin,
                               SHIZSpriteSizeIntrinsic,
                               SHIZSpriteAnchorCenter,
                               SHIZSpriteNoAngle,
                               SHIZSpriteNoTint,
                               SHIZSpriteNoRepeat,
                               false,
                               SHIZLayerDefault);
}

SHIZSize shiz_draw_sprite_ex(SHIZSprite const sprite,
                             SHIZVector2 const origin,
                             SHIZSize const size,
                             SHIZVector2 const anchor,
                             float const angle,
                             SHIZColor const tint,
                             bool const repeat,
                             bool const opaque,
                             SHIZLayer const layer) {
    SHIZSize const sprite_size = shiz_sprite_draw(sprite, origin, size,
                                                  anchor, angle, tint,
                                                  repeat, opaque,
                                                  layer);

#ifdef SHIZ_DEBUG
    if (shiz_debug_context.is_enabled) {
        if (shiz_debug_context.draw_sprite_shape &&
            (sprite_size.width > 0 && sprite_size.height > 0)) {
            _shiz_debug_draw_sprite_shape(origin, sprite_size, anchor, angle, layer);
        }

        if (shiz_debug_context.draw_events) {
            _shiz_debug_event_sprite_resource(origin, shiz_res_get_image(sprite.resource_id));
        }
    }
#endif

    return sprite_size;
}

SHIZSize shiz_measure_sprite_text(SHIZSpriteFont const font,
                                  const char * const text,
                                  SHIZSize const bounds,
                                  SHIZSpriteFontAttributes const attributes) {
    SHIZSpriteFontMeasurement const measurement = shiz_sprite_measure_text(font, text,
                                                                           bounds, attributes);

    return measurement.size;
}

SHIZSize shiz_draw_sprite_text(SHIZSpriteFont const font,
                               const char * const text,
                               SHIZVector2 const origin,
                               SHIZSpriteFontAlignment const alignment) {
    return shiz_draw_sprite_text_ex(font, text, origin, alignment,
                                    SHIZSpriteFontSizeToFit,
                                    SHIZSpriteNoTint,
                                    SHIZSpriteFontAttributesDefault,
                                    SHIZLayerDefault);
}

SHIZSize shiz_draw_sprite_text_ex(SHIZSpriteFont const font,
                                  const char * const text,
                                  SHIZVector2 const origin,
                                  SHIZSpriteFontAlignment const alignment,
                                  SHIZSize const bounds,
                                  SHIZColor const tint,
                                  SHIZSpriteFontAttributes const attributes,
                                  SHIZLayer const layer) {
    return shiz_draw_sprite_text_ex_colored(font, text,
                                            origin, alignment, bounds,
                                            tint, attributes, layer, NULL, 0);
}

SHIZSize shiz_draw_sprite_text_ex_colored(SHIZSpriteFont const font,
                                          const char * const text,
                                          SHIZVector2 const origin,
                                          SHIZSpriteFontAlignment const alignment,
                                          SHIZSize const bounds,
                                          SHIZColor const tint,
                                          SHIZSpriteFontAttributes const attributes,
                                          SHIZLayer const layer,
                                          SHIZColor * const highlight_colors,
                                          uint const highlight_color_count) {
    SHIZSize const text_size = shiz_sprite_draw_text(font, text,
                                                     origin, alignment, bounds,
                                                     tint, attributes, layer,
                                                     highlight_colors, highlight_color_count);

#ifdef SHIZ_DEBUG
    if (shiz_debug_context.is_enabled) {
        if (shiz_debug_context.draw_sprite_shape &&
            (text_size.width > 0 && text_size.height > 0)) {
            SHIZVector2 anchor = SHIZSpriteAnchorCenter;

            if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
                if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
                    anchor = SHIZSpriteAnchorTopLeft;
                } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                    anchor = SHIZSpriteAnchorTop;
                } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                    anchor = SHIZSpriteAnchorTopRight;
                }
            } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
                if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
                    anchor = SHIZSpriteAnchorLeft;
                } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                    anchor = SHIZSpriteAnchorCenter;
                } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                    anchor = SHIZSpriteAnchorRight;
                }
            } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
                if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
                    anchor = SHIZSpriteAnchorBottomLeft;
                } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                    anchor = SHIZSpriteAnchorBottom;
                } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                    anchor = SHIZSpriteAnchorBottomRight;
                }
            }

            _shiz_debug_draw_sprite_shape(origin, text_size, anchor,
                                          SHIZSpriteNoAngle, SHIZLayerDefault);
        }

        if (shiz_debug_context.draw_events) {
            _shiz_debug_event_sprite_resource(origin, shiz_res_get_image(font.sprite.resource_id));
        }
    }
#endif

    return text_size;
}

#ifdef SHIZ_DEBUG
static void _shiz_debug_build_stats() {
    SHIZViewport const viewport = shiz_gfx_get_viewport();

    sprintf(_shiz_debug_stats_buffer,
            "\4%.0fx%.0f\1@\5%.0fx%.0f\1\n\n"
            "\2%0.2fms/frame\1 (\4%0.2fms\1)\n"
            "\2%d\1 (\3%d\1|\4%d\1|\5%d\1)\n\n"
            "%c%d/%d sprites/frame\1\n"
            "\2%d draws/frame\1\n\n"
            "\2\x17%.1fx time\1",
            viewport.screen.width, viewport.screen.height,
            viewport.framebuffer.width, viewport.framebuffer.height,
            shiz_gfx_debug_get_frame_time(),
            shiz_gfx_debug_get_frame_time_avg(),
            shiz_gfx_debug_get_frames_per_second(),
            shiz_gfx_debug_get_frames_per_second_min(),
            shiz_gfx_debug_get_frames_per_second_avg(),
            shiz_gfx_debug_get_frames_per_second_max(),
            (shiz_debug_context.sprite_count > SHIZSpriteInternalMax ? '\3' : '\2'),
            shiz_debug_context.sprite_count, SHIZSpriteInternalMax,
            shiz_gfx_debug_get_draw_count(),
            shiz_get_time_scale());
}

static void _shiz_debug_event(SHIZDebugEvent const event) {
    if (shiz_debug_context.is_tracking_enabled) {
        if (shiz_debug_context.event_count < SHIZDebugEventMax) {
            shiz_debug_context.events[shiz_debug_context.event_count].name = event.name;
            shiz_debug_context.events[shiz_debug_context.event_count].lane = event.lane;
            shiz_debug_context.events[shiz_debug_context.event_count].origin = event.origin;

            shiz_debug_context.event_count += 1;
        } else {
            printf("event not shown: %s", event.name);
        }
    }
}

static void _shiz_debug_draw_events() {
    uint const line_margin = 8;
    uint const lane_margin = 2;
    uint const lane_size = 4;

    uint draw_events = 0;

    SHIZLayer const layer = SHIZLayeredWithDepth(SHIZLayerMax, SHIZLayerDepthMax);
    
    for (uint i = 0; i < shiz_debug_context.event_count; i++) {
        SHIZDebugEvent const event = shiz_debug_context.events[i];

        float const lane_offset = lane_margin + (lane_size * event.lane) + (lane_margin * event.lane);

        SHIZVector2 const from = SHIZVector2Make(event.origin.x,
                                                 shiz_context.preferred_screen_size.height - lane_offset);
        SHIZVector2 const to = SHIZVector2Make(event.origin.x,
                                               event.origin.y);

        SHIZColor tint = SHIZSpriteNoTint;

        char event_buffer[128];

        if (event.lane == SHIZDebugEventLaneDraws) {
            tint = SHIZColorYellow;
            
            sprintf(event_buffer, "%u) %s", draw_events + 1, event.name);

            draw_events += 1;
        } else if (event.lane == SHIZDebugEventLaneResources) {
            sprintf(event_buffer, "%s", event.name);
        }

        _shiz_str_to_upper(event_buffer);

        tint.alpha = 0.6f;

        shiz_draw_line_ex(SHIZVector2Make(from.x, from.y - line_margin), to, tint, layer);

        shiz_draw_sprite_text_ex(shiz_debug_font,
                                 event_buffer,
                                 from,
                                 SHIZSpriteFontAlignmentCenter | SHIZSpriteFontAlignmentTop,
                                 SHIZSpriteFontSizeToFit, tint,
                                 SHIZSpriteFontAttributesWithScale(0.6f), layer);
    }
}

static void _shiz_debug_draw_stats() {
    SHIZLayer const layer = SHIZLayeredWithDepth(SHIZLayerMax, SHIZLayerDepthMax - 1);
    
    uint const margin = 8;

    SHIZColor highlight_colors[] = {
        SHIZColorFromHex(0xefec0d), // yellow
        SHIZColorFromHex(0xe5152d), // red
        SHIZColorFromHex(0x36cd33), // green
        SHIZColorFromHex(0x20b1fc) // blue
    };

    SHIZVector2 stats_text_origin =
    SHIZVector2Make(shiz_context.preferred_screen_size.width - margin,
                    shiz_context.preferred_screen_size.height - margin);

    SHIZSize stats_text_size =
    shiz_draw_sprite_text_ex_colored(shiz_debug_font,
                                     _shiz_debug_stats_buffer,
                                     stats_text_origin,
                                     SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentRight,
                                     SHIZSpriteFontSizeToFit, SHIZSpriteNoTint,
                                     SHIZSpriteFontAttributesDefault, layer,
                                     highlight_colors, 4);

    char version_buffer[128];

    sprintf(version_buffer, "SHIZEN %d.%d.%d / %s (built %s, %s)",
            SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
            SHIZEN_VERSION_NAME, __DATE__, __TIME__);

    shiz_draw_sprite_text_ex(shiz_debug_font,
                             version_buffer,
                             SHIZVector2Make(margin / 2, margin / 2),
                             SHIZSpriteFontAlignmentBottom | SHIZSpriteFontAlignmentLeft,
                             SHIZSpriteFontSizeToFit, SHIZSpriteTintDefaultWithAlpa(0.25f),
                             SHIZSpriteFontAttributesWithScale(1), layer);

    SHIZVector2 help_text_origin =
    SHIZVector2Make(stats_text_origin.x,
                    stats_text_origin.y - (stats_text_size.height + (margin * 3)));
    
    shiz_draw_sprite_text_ex(shiz_debug_font,
                             "SHFT-1 TOGGLE SHAPES\n"
                             "SHFT-2 TOGGLE EVENTS\n"
                             "SHFT-+  TIME CONTROL",
                             help_text_origin,
                             SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentRight,
                             SHIZSpriteFontSizeToFit, SHIZSpriteNoTint,
                             SHIZSpriteFontAttributesDefault, layer);
}

static void _shiz_debug_draw_sprite_gizmo(SHIZVector2 const anchor, float const angle, SHIZLayer const layer) {
    float const axis_length = 5;
    
    SHIZLayer const layer_above = SHIZLayeredAbove(layer);
    
    float const z = _shiz_layer_get_z(layer_above);
    
    SHIZVector3 const origin = SHIZVector3Make(anchor.x, anchor.y, z);
    
    SHIZVertexPositionColor vertices[2];
    
    vertices[0].position = SHIZVector3Zero;
    
    // x axis
    vertices[1].position = SHIZVector3Make(axis_length, 0, z);
    vertices[0].color = SHIZColorBlue;
    vertices[1].color = SHIZColorBlue;
    
    shiz_gfx_render_ex(GL_LINE_STRIP, vertices, 2, origin, angle);
    
    // y axis
    vertices[1].position = SHIZVector3Make(0, axis_length, z);
    vertices[0].color = SHIZColorGreen;
    vertices[1].color = SHIZColorGreen;
    
    shiz_gfx_render_ex(GL_LINE_STRIP, vertices, 2, origin, angle);

    // anchor
    float const anchor_size = 2;
    
    shiz_draw_rect_ex(SHIZRectMake(anchor, SHIZSizeMake(anchor_size, anchor_size)),
                      SHIZColorRed, SHIZSpriteAnchorCenter, angle, layer_above);
}

static void _shiz_debug_draw_sprite_shape(SHIZVector2 const origin,
                                          SHIZSize const size,
                                          SHIZVector2 const anchor,
                                          float const angle,
                                          SHIZLayer const layer) {
    bool const previously_tracking_events = shiz_debug_context.is_tracking_enabled;

    shiz_debug_context.is_tracking_enabled = false;

    SHIZSize const padded_size = SHIZSizeMake(size.width + 0, size.height + 0);
    SHIZVector2 const padded_origin = SHIZVector2Make(origin.x - 0, origin.y - 0);
    
    SHIZLayer const layer_above = SHIZLayeredAbove(layer);
    
    shiz_draw_rect_shape_ex(SHIZRectMake(padded_origin, padded_size),
                            SHIZColorRed, anchor, angle, layer_above);

    _shiz_debug_draw_sprite_gizmo(origin, angle, layer_above);

    shiz_debug_context.is_tracking_enabled = previously_tracking_events;
}

static void _shiz_debug_event_sprite_resource(SHIZVector2 const origin,
                                              SHIZResourceImage const resource) {
    if (resource.filename) {
        SHIZDebugEvent event;

        event.name = resource.filename;
        event.origin = SHIZVector3Make(origin.x, origin.y, 0);
        event.lane = 1;
        
        _shiz_debug_event(event);
    }
}

static void _shiz_debug_process_errors() {
    GLenum error;

    while ((error = glGetError()) != GL_NO_ERROR) {
        shiz_io_error_context("OPENGL", "%d", error);
    }
}
#endif
