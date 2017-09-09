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

#ifndef draw_h
#define draw_h

#include "type.h"

/**
 * @brief Begin drawing to the screen.
 *
 * Prepare the drawing pipeline for rendering to the graphics context.
 */
void shiz_drawing_begin(SHIZColor const background);
/**
 * @brief End drawing to the screen.
 *
 * Flush the drawing pipeline and present the frame to the graphics context.
 */
void shiz_drawing_end(void);

/**
 * @brief Draw a line.
 */
void shiz_draw_line(SHIZVector2 const from,
                    SHIZVector2 const to,
                    SHIZColor const color);
void shiz_draw_line_ex(SHIZVector2 const from,
                       SHIZVector2 const to,
                       SHIZColor const color,
                       SHIZLayer const layer);
/**
 * @brief Draw a path.
 */
void shiz_draw_path(SHIZVector2 const points[],
                    unsigned int const count,
                    SHIZColor const color);
void shiz_draw_path_ex(SHIZVector2 const points[],
                       unsigned int const count,
                       SHIZColor const color,
                       SHIZLayer const layer);
/**
 * @brief Draw a point.
 */
void shiz_draw_point(SHIZVector2 const point,
                     SHIZColor const color);
void shiz_draw_point_ex(SHIZVector2 const point,
                        SHIZColor const color,
                        SHIZLayer const layer);
/**
 * @brief Draw a rectangle.
 */
void shiz_draw_rect(SHIZRect const rect,
                    SHIZColor const color,
                    SHIZDrawMode const mode);
/**
 * @brief Draw a rectangle.
 */
void shiz_draw_rect_ex(SHIZRect const rect,
                       SHIZColor const color,
                       SHIZDrawMode const mode,
                       SHIZVector2 const anchor,
                       float const angle,
                       SHIZLayer const layer);
/**
 * @brief Draw a circle.
 */
void shiz_draw_circle(SHIZVector2 const center,
                      SHIZColor const color,
                      SHIZDrawMode const mode,
                      float const radius,
                      unsigned int const segments);
void shiz_draw_circle_ex(SHIZVector2 const center,
                         SHIZColor const color,
                         SHIZDrawMode const mode,
                         float const radius,
                         unsigned int const segments,
                         SHIZLayer const layer);

/**
 * @brief Draw an arc.
 */
void shiz_draw_arc(SHIZVector2 const center,
                   SHIZColor const color,
                   SHIZDrawMode const mode,
                   float const radius,
                   unsigned int const segments,
                   float const angle);
void shiz_draw_arc_ex(SHIZVector2 const center,
                      SHIZColor const color,
                      SHIZDrawMode const mode,
                      float const radius,
                      unsigned int const segments,
                      float const angle,
                      SHIZLayer const layer);

/**
 * @brief Draw a sprite.
 *
 * @param sprite
 *        The sprite to draw
 * @param origin
 *        The location where the sprite will be drawn
 *
 * @return a SHIZSize with the bounding width and height of the drawn sprite
 */
SHIZSize shiz_draw_sprite(SHIZSprite const sprite,
                          SHIZVector2 const origin);
/**
 * @brief Draw a sprite.
 *
 * @param sprite
 *        The sprite to draw
 * @param origin
 *        The location where the sprite will be drawn
 * @param size
 *        The size to draw the sprite with (unless `repeat` is true, the sprite is scaled to fit).
 *        Use `SHIZSpriteSizeIntrinsic` to draw this sprite at its default size.
 * @param anchor
 *        Sets an anchor that defines where on the sprite the origin is.
 *        The anchor also defines the pivot point for any applied rotation.
 *        The anchor is relative to the entire sprite; e.g. { 0, 0 } defines a pivot
 *        at the center, { -1, 0 } at the middle-left of the sprite, and { 1, 0 } defines
 *        a pivot at the middle-right.
 *        Similarly, { 1, 1 } defines a pivot at the top-right, and { -1, 1 } at the top-left
 * @param angle
 *        The angle in radians to rotate the sprite by (rotation is applied on
 *        the pivot point specified by the `anchor` parameter)
 * @param tint
 *        The color to tint the sprite with (default blending mode is multiplicative)
 * @param repeat
 *        Specify whether the sprite should repeat (only applies if the `size` parameter
 *        sets a size larger than the intrinsic size of the sprite)
 * @param opaque
 *        Specify whether the sprite does not draw transparent pixels
 * @param layer
 *        The layer to draw this sprite in (lower layers are drawn first)
 *
 * @return a SHIZSize with the bounding width and height of the drawn sprite
 */
SHIZSize shiz_draw_sprite_ex(SHIZSprite const sprite,
                             SHIZVector2 const origin,
                             SHIZSpriteSize const size,
                             SHIZVector2 const anchor,
                             float const angle,
                             SHIZColor const tint,
                             bool const repeat,
                             bool const opaque,
                             SHIZLayer const layer);

/**
 * @brief Measure the size of a text.
 *
 * Measure the bounding width and height of a text.
 *
 * @param font
 *        A SHIZSpriteFont defining the resource that would be used to draw the text with
 * @param text
 *        The string of text to measure
 * @param bounds
 *        A SHIZSize with bounds that the text must not exceed; text that would exceed these
 *        bounds will be truncated instead (use `SHIZSpriteFontSizeToFit` to disable bounds
 *        entirely, or e.g. `SHIZSizeMake(200, SHIZSpriteFontSizeToFitVertically)` to only
 *        apply horizontal bounds- or vice versa)
 * @param attributes
 *        A SHIZSpriteFontAttributes containing any additional attributes that would
 *        be used when drawing the text
 *
 * @return a SHIZSize with the bounding width and height
 */
SHIZSize shiz_measure_sprite_text(SHIZSpriteFont const font,
                                  const char * const text,
                                  SHIZSize const bounds,
                                  SHIZSpriteFontAttributes const attributes);

/**
 * @brief Draw a text at a location.
 *
 * Draw a text at a location, aligned as specified.
 *
 * @param font
 *        A SHIZSpriteFont defining the resource containing character sprites
 * @param text
 *        The string of text to draw
 * @param origin
 *        The location where the text will be drawn
 * @param alignment
 *        A SHIZSpriteFontAlignment defining the orientation of the text
 *        (defaults to `SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentLeft`)
 *
 * @return a SHIZSize with the bounding width and height of the drawn text
 */
SHIZSize shiz_draw_sprite_text(SHIZSpriteFont const font,
                               const char * const text,
                               SHIZVector2 const origin,
                               SHIZSpriteFontAlignment const alignment);
/**
 * @brief Draw text at a location.
 *
 * Draw text at a location, aligned and attributed as specified.
 *
 * @remark Applying vertical bounds can cause the text to be truncated if it exceeds the bounds.
 *
 * @param font
 *        A SHIZSpriteFont defining the resource containing character sprites
 * @param text
 *        The string of text to draw
 * @param origin
 *        The location where the text will be drawn
 * @param alignment
 *        A SHIZSpriteFontAlignment defining the orientation of the text
 *        (defaults to `SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentLeft`)
 * @param bounds
 *        A SHIZSize with bounds that the text must not exceed; text that would exceed these
 *        bounds will be truncated instead (set `SHIZSpriteFontSizeToFit` to disable bounds
 *        entirely, or e.g. `SHIZSizeMake(200, SHIZSpriteFontSizeToFitVertically)` to only
 *        apply horizontal bounds- or vice versa)
 * @param tint
 *        A SHIZColor that is applied on each drawn character sprite
 *        (specify `SHIZSpriteNoTint` to disable tinting)
 * @param attributes
 *        A SHIZSpriteFontAttributes containing any additional attributes to draw the text
 *        (use `SHIZSpriteFontAttributesDefault` to apply default attributes)
 * @param layer
 *        The layer to draw text in
 *
 * @return a SHIZSize with the bounding width and height of the drawn text
 */
SHIZSize shiz_draw_sprite_text_ex(SHIZSpriteFont const font,
                                  const char * const text,
                                  SHIZVector2 const origin,
                                  SHIZSpriteFontAlignment const alignment,
                                  SHIZSize const bounds,
                                  SHIZColor const tint,
                                  SHIZSpriteFontAttributes const attributes,
                                  SHIZLayer const layer);
/**
 * @brief Draw highlighted text at a location.
 */
SHIZSize shiz_draw_sprite_text_ex_colored(SHIZSpriteFont const font,
                                          const char * const text,
                                          SHIZVector2 const origin,
                                          SHIZSpriteFontAlignment const alignment,
                                          SHIZSize const bounds,
                                          SHIZColor const tint,
                                          SHIZSpriteFontAttributes const attributes,
                                          SHIZLayer const layer,
                                          SHIZColor * const highlight_colors,
                                          unsigned int const highlight_color_count);

#endif // draw_h
