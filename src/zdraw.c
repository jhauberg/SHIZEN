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

#include <SHIZEN/zdraw.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "internal.h"

#include "sprite.h"
#include "spritefont.h"

#include "graphics/gfx.h"

#ifdef SHIZ_DEBUG
 #include "debug/debug.h"
 #include "res.h"
#endif

extern SHIZGraphicsContext const _graphics_context;

static void z_draw__path_3d(SHIZVector3 const points[],
                            unsigned int const count,
                            SHIZColor const color);

static void z_draw__line_3d(SHIZVector3 const from,
                               SHIZVector3 const to,
                               SHIZColor const color);

static void z_draw__flush(void);

void
z_drawing_begin(SHIZColor const background)
{
    z_sprite__reset();

    z_gfx__begin(background);

#ifdef SHIZ_DEBUG
    z_debug__reset_events();
#endif
}

void
z_drawing_end()
{
    z_draw__flush();

#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        bool const previously_drawing_shapes = z_debug__is_drawing_shapes();
        bool const previously_enabled_events = z_debug__is_events_enabled();

        z_debug__set_drawing_shapes(false);
        z_debug__set_events_enabled(false);

        z_debug__draw_viewport();
        z_debug__build_stats();
        z_debug__draw_stats();

        if (z_debug__is_drawing_events()) {
            z_debug__draw_events();
        }

        z_draw__flush();

        z_debug__set_drawing_shapes(previously_drawing_shapes);
        z_debug__set_events_enabled(previously_enabled_events);
    }
#endif

    z_gfx__end();

#ifdef SHIZ_DEBUG
    // flush out any encountered errors during this frame
    z_debug__process_errors();
#endif
    
    z_engine__present_frame();
}

void
z_draw_line(SHIZVector2 const from,
            SHIZVector2 const to,
            SHIZColor const color)
{
    z_draw_line_ex(from, to, color, SHIZLayerDefault);
}

void
z_draw_line_ex(SHIZVector2 const from,
               SHIZVector2 const to,
               SHIZColor const color,
               SHIZLayer const layer)
{
    float const z = z_layer__get_z(layer);
    
    z_draw__line_3d(SHIZVector3Make(from.x, from.y, z),
                    SHIZVector3Make(to.x, to.y, z),
                    color);
}

void
z_draw_path(SHIZVector2 const points[],
            unsigned int const count,
            SHIZColor const color)
{
    z_draw_path_ex(points, count, color, SHIZLayerDefault);
}

void
z_draw_path_ex(SHIZVector2 const points[],
               unsigned int const count,
               SHIZColor const color,
               SHIZLayer const layer)
{
    SHIZVector3 points_3d[count];
    
    float const z = z_layer__get_z(layer);
    
    for (unsigned int i = 0; i < count; i++) {
        points_3d[i].x = points[i].x;
        points_3d[i].y = points[i].y;
        points_3d[i].z = z;
    }
    
    z_draw__path_3d(points_3d, count, color);
}

void
z_draw_point(SHIZVector2 const point,
             SHIZColor const color)
{
    z_draw_point_ex(point, color, SHIZLayerDefault);
}

void
z_draw_point_ex(SHIZVector2 const point,
                SHIZColor const color,
                SHIZLayer const layer)
{
    SHIZVertexPositionColor vertices[1] = {
        {
            .position = SHIZVector3Make(point.x, point.y,
                                        z_layer__get_z(layer)),
            .color = color
        }
    };

    z_gfx__render(GL_POINTS, vertices, 1);
}

void
z_draw_rect(SHIZRect const rect,
            SHIZColor const color,
            SHIZDrawMode const mode)
{
    z_draw_rect_ex(rect, color, mode,
                   SHIZAnchorBottomLeft,
                   SHIZSpriteNoAngle,
                   SHIZLayerDefault);
}

void
z_draw_rect_ex(SHIZRect const rect,
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

    float const z = z_layer__get_z(layer);

    SHIZVector3 const origin = SHIZVector3Make(rect.origin.x,
                                               rect.origin.y,
                                               z);

    SHIZRect const anchored_rect = z_sprite__anchor_rect(rect.size,
                                                         anchor);

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
        z_gfx__render_ex(GL_TRIANGLE_STRIP,
                         vertices, vertex_count,
                         origin, angle);
    } else {
        z_gfx__render_ex(GL_LINE_LOOP,
                         vertices, vertex_count,
                         origin, angle);
    }
}

void
z_draw_circle(SHIZVector2 const center,
              SHIZColor const color,
              SHIZDrawMode const mode,
              float const radius,
              unsigned int const segments)
{
    z_draw_circle_ex(center, color, mode, radius, segments,
                     SHIZLayerDefault);
}

void
z_draw_circle_ex(SHIZVector2 const center,
                 SHIZColor const color,
                 SHIZDrawMode const mode,
                 float const radius,
                 unsigned int const segments,
                 SHIZLayer const layer)
{
    unsigned int const vertex_count = mode == SHIZDrawModeFill ?
        (segments + 2) : segments;

    SHIZVertexPositionColor vertices[vertex_count];

    float const z = z_layer__get_z(layer);
    float const step = 2.0f * (float)M_PI / segments;

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
            // connect the last vertex to the first shape vertex
            // (i.e. not center in case of fill)
            unsigned int const last_index = vertex_offset + segments;

            vertices[last_index].color = color;
            vertices[last_index].position = vertices[vertex_index].position;
        }
    }

    if (mode == SHIZDrawModeFill) {
        z_gfx__render_ex(GL_TRIANGLE_FAN, vertices, vertex_count, origin,
                         SHIZSpriteNoAngle);
    } else {
        z_gfx__render_ex(GL_LINE_LOOP, vertices, vertex_count, origin,
                         SHIZSpriteNoAngle);
    }
}

void
z_draw_arc(SHIZVector2 const center,
           SHIZColor const color,
           SHIZDrawMode const mode,
           float const radius,
           unsigned int const segments,
           float const angle)
{
    z_draw_arc_ex(center, color, mode, radius, segments, angle,
                  SHIZLayerDefault);
}

void
z_draw_arc_ex(SHIZVector2 const center,
              SHIZColor const color,
              SHIZDrawMode const mode,
              float const radius,
              unsigned int const segments,
              float const angle,
              SHIZLayer const layer)
{
    unsigned int const vertex_count = segments + 2;

    SHIZVertexPositionColor vertices[vertex_count];

    float const target_angle = fmodf(angle, (float)M_PI * 2.0f);

    float const z = z_layer__get_z(layer);
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
        z_gfx__render_ex(GL_TRIANGLE_FAN,
                         vertices, vertex_count,
                         origin, SHIZSpriteNoAngle);
    } else {
        z_gfx__render_ex(GL_LINE_LOOP,
                         vertices, vertex_count,
                         origin, SHIZSpriteNoAngle);
    }
}

SHIZSize
z_draw_sprite(SHIZSprite const sprite,
              SHIZVector2 const origin,
              SHIZSpriteDrawParameters const params)
{
    return z_draw_sprite_ex(sprite, origin,
                            SHIZSpriteSizeIntrinsic,
                            SHIZSpriteNoRepeat,
                            params.anchor,
                            params.angle,
                            params.tint,
                            params.is_opaque,
                            params.layer);
}

SHIZSize z_draw_sprite_sized(SHIZSprite sprite,
                             SHIZVector2 origin,
                             SHIZSpriteSize size,
                             SHIZSpriteDrawParameters params)
{
    return z_draw_sprite_ex(sprite, origin,
                            size,
                            SHIZSpriteNoRepeat,
                            params.anchor,
                            params.angle,
                            params.tint,
                            params.is_opaque,
                            params.layer);
}

SHIZSize z_draw_sprite_tiled(SHIZSprite sprite,
                             SHIZVector2 origin,
                             SHIZSpriteSize size,
                             SHIZSpriteDrawParameters params)
{
    return z_draw_sprite_ex(sprite, origin,
                            size,
                            SHIZSpriteRepeat,
                            params.anchor,
                            params.angle,
                            params.tint,
                            params.is_opaque,
                            params.layer);
}

SHIZSize
z_draw_sprite_ex(SHIZSprite const sprite,
                 SHIZVector2 const origin,
                 SHIZSpriteSize const size,
                 bool const repeat,
                 SHIZVector2 const anchor,
                 float const angle,
                 SHIZColor const tint,
                 bool const opaque,
                 SHIZLayer const layer)
{
    SHIZSize const sprite_size = z_sprite__draw(sprite, origin, size, repeat,
                                                anchor, angle, tint, opaque,
                                                layer);

#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        if (z_debug__is_drawing_shapes() &&
            (sprite_size.width > 0 && sprite_size.height > 0)) {
            z_debug__draw_sprite_shape(origin, sprite_size, SHIZColorRed,
                                       anchor, angle, layer);
        }

        z_debug__add_event_resource(z_res__image(sprite.resource_id).filename,
                                    SHIZVector3Make(origin.x, origin.y, 0));
    }
#endif

    return sprite_size;
}

SHIZSize
z_measure_text(SHIZSpriteFont const font,
               char const * const text)
{
    return z_measure_text_attributed(font, text,
                                     SHIZSpriteFontSizeToFit,
                                     SHIZSpriteFontAttributesDefault);
}

SHIZSize
z_measure_text_attributed(SHIZSpriteFont const font,
                          char const * const text,
                          SHIZSize const bounds,
                          SHIZSpriteFontAttributes const attributes)
{
    SHIZSpriteFontMeasurement const measurement =
        z_sprite__measure_text(font, text, bounds, attributes);
    
    return measurement.size;
}

SHIZSize
z_draw_text(SHIZSpriteFont const font,
            char const * const text,
            SHIZVector2 const origin,
            SHIZSpriteFontDrawParameters const params)
{
    SHIZSize const bounds = SHIZSpriteFontSizeToFit;
    
    return z_draw_text_bounded(font, text, origin, bounds, params);
}

SHIZSize z_draw_text_bounded(SHIZSpriteFont font,
                             char const * text,
                             SHIZVector2 origin,
                             SHIZSize bounds,
                             SHIZSpriteFontDrawParameters params)
{
    return z_draw_text_attributed(font, text, origin, bounds,
                                  SHIZSpriteFontAttributesDefault,
                                  params);
}

SHIZSize z_draw_text_attributed(SHIZSpriteFont font,
                                char const * text,
                                SHIZVector2 origin,
                                SHIZSize bounds,
                                SHIZSpriteFontAttributes attribs,
                                SHIZSpriteFontDrawParameters params)
{
    return z_draw_text_ex(font, text, origin, bounds, attribs,
                          params.alignment, params.tint, params.layer);
}

SHIZSize
z_draw_text_ex(SHIZSpriteFont const font,
               const char * const text,
               SHIZVector2 const origin,
               SHIZSize const bounds,
               SHIZSpriteFontAttributes const attributes,
               SHIZSpriteFontAlignment const alignment,
               SHIZColor const tint,
               SHIZLayer const layer)
{
    SHIZSize const text_size =
        z_sprite__draw_text(font, text,
                            origin, alignment, bounds,
                            attributes, tint, layer);
    
#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        if (z_debug__is_drawing_shapes() &&
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
            z_debug__draw_sprite_shape(origin, text_size, SHIZColorRed, anchor,
                                       SHIZSpriteNoAngle,
                                       layer);
            
            if (bounds.width > 0 && bounds.height > 0) {
                // draw bounds
                z_debug__draw_sprite_shape(origin, bounds, SHIZColorYellow, anchor,
                                           SHIZSpriteNoAngle,
                                           layer);
            }
        }
        
        z_debug__add_event_resource(z_res__image(font.sprite.resource_id).filename,
                                    SHIZVector3Make(origin.x, origin.y, 0));
    }
#endif
    
    return text_size;
}

static
void
z_draw__path_3d(SHIZVector3 const points[],
                unsigned int const count,
                SHIZColor const color)
{
    SHIZVertexPositionColor vertices[count];
    
    for (unsigned int i = 0; i < count; i++) {
        vertices[i].position = points[i];
        vertices[i].color = color;
    }
    
    z_gfx__render(GL_LINE_STRIP, vertices, count);
}

static
void
z_draw__line_3d(SHIZVector3 const from,
                SHIZVector3 const to,
                SHIZColor const color)
{
    SHIZVector3 points[] = {
        from, to
    };
    
    z_draw__path_3d(points, 2, color);
}

static
void
z_draw__flush()
{
    z_sprite__flush();
    z_gfx__flush();
}
