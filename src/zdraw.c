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

#include <math.h> // M_PI, cosf, sinf, fmodf

#include "internal.h"

#include "sprite.h"
#include "spritefont.h"

#include "graphics/gfx.h"

#ifdef SHIZ_DEBUG
 #include "debug/debug.h"
 #include "res.h"
#endif

extern SHIZGraphicsContext const _graphics_context;
extern SHIZSprite const _spr_white_1x1;

static
void
z_draw__flush(void);

static
void
z_draw__rect(SHIZRect rect,
             SHIZColor color,
             SHIZVector2 anchor,
             f32 angle,
             SHIZLayer layer);

static
void
z_draw__rect_outline(SHIZRect rect,
                     SHIZColor color,
                     SHIZVector2 anchor,
                     f32 angle,
                     SHIZLayer layer);

static
SHIZVector2
z_draw__pixel_centering_offset(SHIZVector2 const anchor)
{
    return SHIZVector2Make(HALF_PIXEL * -anchor.x,
                           HALF_PIXEL * -anchor.y);
}

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

        z_debug__build_stats();
        z_debug__draw_stats();

        z_debug__draw_viewport();
        
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
    SHIZVector2 points[] = {
        from, to
    };
    
    z_draw_path_ex(points, 2, color, layer);
}

void
z_draw_path(SHIZVector2 const points[],
            u16 const count,
            SHIZColor const color)
{
    z_draw_path_ex(points, count, color, SHIZLayerDefault);
}

void
z_draw_path_ex(SHIZVector2 const points[],
               u16 const count,
               SHIZColor const color,
               SHIZLayer const layer)
{
    f32 const z = z_layer__get_z(layer);
    
    SHIZVertexPositionColor vertices[count];
    
    SHIZVector2 const anchor = SHIZAnchorBottomLeft;
    SHIZVector2 const offset = z_draw__pixel_centering_offset(anchor);
    
    for (u8 i = 0; i < count; i++) {
        SHIZVector2 const point = points[i];
        
        vertices[i].position = SHIZVector3Make(PIXEL(point.x) + offset.x,
                                               PIXEL(point.y) + offset.y,
                                               z);
        
        vertices[i].color = color;
    }
    
    z_gfx__render(GL_LINE_STRIP, vertices, count);
    
#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        if (z_debug__is_drawing_shapes() && count > 1) {
            z_debug__draw_path_bounds(points, count, SHIZColorRed, layer);
        }
    }
#endif
}

void
z_draw_point(SHIZVector2 const point,
             SHIZColor const color)
{
    z_draw_point_anchored(point, color, SHIZAnchorBottomLeft);
}

void
z_draw_point_anchored(SHIZVector2 const point,
                      SHIZColor const color,
                      SHIZVector2 const anchor)
{
    z_draw_point_ex(point, color, anchor, SHIZLayerDefault);
}

void
z_draw_point_ex(SHIZVector2 const point,
                SHIZColor const color,
                SHIZVector2 const anchor,
                SHIZLayer const layer)
{
    // we can utilize the spritebatch to draw 1x1 sprites instead of rendering
    // vertices using GL_POINTS
    SHIZSpriteParameters params = SHIZSpriteParametersDefault(); {
        params.tint = color;
        params.layer = layer;
        params.anchor = anchor;
    }
    
    bool const was_drawing_debug_shapes = z_debug__is_drawing_shapes();
    
    // we don't want debug shapes on points
    z_debug__set_drawing_shapes(false);
    
    z_draw_sprite(_spr_white_1x1, SHIZVector2Make(point.x, point.y), params);
    
    z_debug__set_drawing_shapes(was_drawing_debug_shapes);
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
               f32 const angle,
               SHIZLayer const layer)
{
    if (mode == SHIZDrawModeFill) {
        z_draw__rect(rect, color, anchor, angle, layer);
    } else {
        z_draw__rect_outline(rect, color, anchor, angle, layer);
    }
}

void
z_draw_circle(SHIZVector2 const center,
              SHIZColor const color,
              SHIZDrawMode const mode,
              f32 const radius,
              u8 const segments)
{
    z_draw_circle_scaled(center, color, mode, radius, segments,
                         SHIZVector2One);
}

void
z_draw_circle_scaled(SHIZVector2 const center,
                     SHIZColor const color,
                     SHIZDrawMode const mode,
                     f32 const radius,
                     u8 const segments,
                     SHIZVector2 const scale)
{
    z_draw_circle_ex(center, color, mode, radius, segments,
                     scale, SHIZLayerDefault);
}

void
z_draw_circle_ex(SHIZVector2 const center,
                 SHIZColor const color,
                 SHIZDrawMode const mode,
                 f32 const radius,
                 u8 const segments,
                 SHIZVector2 const scale,
                 SHIZLayer const layer)
{
    u16 const vertex_count = mode == SHIZDrawModeFill ?
        (segments + 2) : segments;

    SHIZVertexPositionColor vertices[vertex_count];

    f32 const z = z_layer__get_z(layer);
    f32 const step = 2.0f * (f32)M_PI / segments;

    SHIZVector2 const offset = z_draw__pixel_centering_offset(SHIZAnchorCenter);
    
    SHIZVector3 const origin = SHIZVector3Make(PIXEL(center.x) + offset.x,
                                               PIXEL(center.y) + offset.y,
                                               z);
    
    u8 vertex_offset = 0;

    if (mode == SHIZDrawModeFill) {
        // start at the center
        vertices[0].color = color;
        vertices[0].position = SHIZVector3Zero;

        vertex_offset = 1;
    }

    for (u8 segment = 0; segment < segments; segment++) {
        u16 const vertex_index = vertex_offset + segment;

        f32 const angle = segment * -step;

        f32 const x = (radius * scale.x) * cosf(angle);
        f32 const y = (radius * scale.y) * sinf(angle);

        vertices[vertex_index].color = color;
        // note that we're intentionally leaving out the 'z' here
        vertices[vertex_index].position = SHIZVector3Make(x, y, 0);

        if (mode == SHIZDrawModeFill && segment == 0) {
            // connect the last vertex to the first shape vertex
            // (i.e. not center in case of fill)
            u16 const last_index = vertex_offset + segments;

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
    
#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        if (z_debug__is_drawing_shapes() && radius > 0) {
            z_debug__draw_circle_bounds(center, SHIZColorRed,
                                        radius, scale, layer);
        }
    }
#endif
}

void
z_draw_arc(SHIZVector2 const center,
           SHIZColor const color,
           SHIZDrawMode const mode,
           f32 const radius,
           u8 const segments,
           f32 const angle)
{
    z_draw_arc_ex(center, color, mode, radius, segments, angle,
                  SHIZLayerDefault);
}

void
z_draw_arc_ex(SHIZVector2 const center,
              SHIZColor const color,
              SHIZDrawMode const mode,
              f32 const radius,
              u8 const segments,
              f32 const angle,
              SHIZLayer const layer)
{
    u16 const vertex_count = segments + 2;

    SHIZVertexPositionColor vertices[vertex_count];

    f32 const target_angle = fmodf(angle, (f32)M_PI * 2.0f);

    f32 const z = z_layer__get_z(layer);
    f32 const step = target_angle / segments;

    SHIZVector2 const offset = z_draw__pixel_centering_offset(SHIZAnchorCenter);
    
    SHIZVector3 const origin = SHIZVector3Make(PIXEL(center.x) + offset.x,
                                               PIXEL(center.y) + offset.y,
                                               z);
    
    u8 vertex_offset = 1;

    vertices[0].color = color;
    vertices[0].position = SHIZVector3Zero;

    u16 end = segments + 1;
    
    for (u8 segment = 0; segment < end; segment++) {
        u16 const vertex_index = vertex_offset + segment;

        f32 const step_angle = segment * -step;

        f32 const x = radius * cosf(step_angle);
        f32 const y = radius * sinf(step_angle);

        vertices[vertex_index].color = color;
        // note that we're intentionally leaving out the 'z' here
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
              SHIZSpriteParameters const params)
{
    return z_draw_sprite_ex(sprite, origin,
                            SHIZSpriteSizeIntrinsic,
                            SHIZSpriteNoRepeat,
                            params.anchor,
                            params.flip,
                            params.angle,
                            params.tint,
                            params.is_opaque,
                            params.layer);
}

SHIZSize
z_draw_sprite_sized(SHIZSprite sprite,
                    SHIZVector2 origin,
                    SHIZSpriteSize size,
                    SHIZSpriteParameters params)
{
    return z_draw_sprite_ex(sprite, origin,
                            size,
                            SHIZSpriteNoRepeat,
                            params.anchor,
                            params.flip,
                            params.angle,
                            params.tint,
                            params.is_opaque,
                            params.layer);
}

SHIZSize
z_draw_sprite_tiled(SHIZSprite sprite,
                    SHIZVector2 origin,
                    SHIZSpriteSize size,
                    SHIZSpriteParameters params)
{
    return z_draw_sprite_ex(sprite, origin,
                            size,
                            SHIZSpriteRepeat,
                            params.anchor,
                            params.flip,
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
                 SHIZSpriteFlipMode const flip,
                 f32 const angle,
                 SHIZColor const tint,
                 bool const opaque,
                 SHIZLayer const layer)
{
    SHIZSize const sprite_size = z_sprite__draw(sprite,
                                                origin,
                                                size, repeat,
                                                anchor, flip, angle, tint,
                                                opaque, layer);

#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        if (z_debug__is_drawing_shapes() && (sprite_size.width > 0 &&
                                             sprite_size.height > 0)) {
            z_debug__draw_sprite_bounds(origin, sprite_size, SHIZColorRed,
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
        z_spritefont__measure_text(font, text, bounds, attributes);
    
    return measurement.size;
}

SHIZSize
z_draw_text(SHIZSpriteFont const font,
            char const * const text,
            SHIZVector2 const origin,
            SHIZSpriteFontParameters const params)
{
    SHIZSize const bounds = SHIZSpriteFontSizeToFit;
    
    return z_draw_text_bounded(font, text, origin, bounds, params);
}

SHIZSize z_draw_text_bounded(SHIZSpriteFont font,
                             char const * text,
                             SHIZVector2 origin,
                             SHIZSize bounds,
                             SHIZSpriteFontParameters params)
{
    return z_draw_text_attributed(font, text, origin, bounds,
                                  SHIZSpriteFontAttributesDefault,
                                  params);
}

SHIZSize
z_draw_text_attributed(SHIZSpriteFont font,
                       char const * text,
                       SHIZVector2 origin,
                       SHIZSize bounds,
                       SHIZSpriteFontAttributes attribs,
                       SHIZSpriteFontParameters params)
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
        z_spritefont__draw_text(font, text,
                                SHIZVector2Make(PIXEL(origin.x),
                                                PIXEL(origin.y)),
                                alignment, bounds,
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
            z_debug__draw_sprite_bounds(origin, text_size, SHIZColorRed,
                                        anchor, SHIZSpriteNoAngle,
                                        layer);
            
            if (bounds.width > 0 && bounds.height > 0) {
                // draw bounds
                z_debug__draw_sprite_bounds(origin, bounds, SHIZColorYellow,
                                           anchor, SHIZSpriteNoAngle,
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
z_draw__rect_outline(SHIZRect const rect,
                     SHIZColor const color,
                     SHIZVector2 const anchor,
                     f32 const angle,
                     SHIZLayer const layer)
{
    u8 const vertex_count = 4;
    
    SHIZVertexPositionColor vertices[vertex_count];
    
    for (u8 i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
    }
    
    f32 const z = z_layer__get_z(layer);
    
    SHIZVector2 const offset = z_draw__pixel_centering_offset(anchor);
    
    SHIZVector3 const origin = SHIZVector3Make(PIXEL(rect.origin.x) + offset.x,
                                               PIXEL(rect.origin.y) + offset.y,
                                               z);
    
    // because of how pixels are rasterized, we need to make the box 1px smaller
    // this achieves the result that we want; e.g. a rect (0,0) at 3x3, paints
    // exactly 3 pixels wide, and 3 pixels tall (depending on anchoring) when
    // it's size is considered as only 2x2
    SHIZRect const anchored =
        z_sprite__anchor_rect(SHIZSizeMake(rect.size.width - 1,
                                           rect.size.height - 1),
                              anchor);
    
    f32 const l = PIXEL(anchored.origin.x);
    f32 const r = PIXEL(anchored.origin.x + anchored.size.width);
    f32 const b = PIXEL(anchored.origin.y);
    f32 const t = PIXEL(anchored.origin.y + anchored.size.height);
    
    // note that we're intentionally leaving out the 'z' for the vertices,
    // as each vertex will be transformed by the Z of the origin later and
    // thus becomes unnecessary to add here
    vertices[0].position = SHIZVector3Make(l, b, 0);
    vertices[1].position = SHIZVector3Make(l, t, 0);
    vertices[2].position = SHIZVector3Make(r, t, 0);
    vertices[3].position = SHIZVector3Make(r, b, 0);
    
    z_gfx__render_ex(GL_LINE_LOOP,
                     vertices, vertex_count,
                     origin, angle);
    
#ifdef SHIZ_DEBUG
    if (z_debug__is_enabled()) {
        if (z_debug__is_drawing_shapes() && (anchored.size.width > 0 &&
                                             anchored.size.height > 0)) {
            z_debug__draw_rect_bounds(rect, SHIZColorRed,
                                      anchor, angle, layer);
        }
    }
#endif
}

static
void
z_draw__rect(SHIZRect const rect,
             SHIZColor const color,
             SHIZVector2 const anchor,
             f32 const angle,
             SHIZLayer const layer)
{
    // we can utilize the spritebatch when drawing filled rectangles
    // by stretching a 1x1 white texture to the size of our rect
    SHIZSpriteParameters params = SHIZSpriteParametersDefault(); {
        params.angle = angle;
        params.tint = color;
        params.layer = layer;
        params.anchor = anchor;
    }
    
    // note that this automatically triggers a debug bounds draw
    z_draw_sprite_sized(_spr_white_1x1,
                        rect.origin,
                        SHIZSpriteSized(rect.size, SHIZVector2One),
                        params);
}

static
void
z_draw__flush()
{
    z_sprite__flush();
    z_gfx__flush();
}
