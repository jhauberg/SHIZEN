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

#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "internal.h"
#include "gfx.h"
#include "res.h"
#include "io.h"

static SHIZRect _shiz_get_anchored_rect(SHIZSize const size, SHIZVector2 const anchor);

static void _shiz_draw_rect(SHIZRect const rect, SHIZColor const color, bool const fill, SHIZVector2 const anchor, float const angle);

static void shiz_draw_path_3d(SHIZVector3 const points[], uint const count, SHIZColor const color);
static void shiz_draw_line_3d(SHIZVector3 const from, SHIZVector3 const to, SHIZColor const color);

static SHIZSpriteFontMeasurement _shiz_measure_sprite_text(SHIZSpriteFont const font, const char * const text, SHIZSize const bounds, SHIZSpriteFontAttributes const attributes);

static int _shiz_compare_sprites(const void * a, const void * b);
static void _shiz_flush_sprites(void);

static uint _shiz_sprites_count = 0;

static SHIZSpriteInternal _shiz_sprites[SHIZSpriteInternalMax];

static unsigned short const _shiz_sprite_layer_min = 0;
static unsigned short const _shiz_sprite_layer_max = USHRT_MAX;

#ifdef SHIZ_DEBUG
static void _shiz_debug_process_errors(void);
static void _shiz_debug_build_stats(void);
static void _shiz_debug_display_stats(void);

static char _shiz_debug_font_buffer[128];
#endif

void shiz_drawing_begin() {
    shiz_gfx_clear();
    shiz_gfx_begin();
}

void shiz_drawing_end() {
#ifdef SHIZ_DEBUG
    if (shiz_context.is_debug_enabled) {
        _shiz_debug_display_stats();
    }
#endif

    _shiz_flush_sprites();

    shiz_gfx_end();

#ifdef SHIZ_DEBUG
    _shiz_debug_build_stats();
    _shiz_debug_process_errors();
#endif

    glfwSwapBuffers(shiz_context.window);
    glfwPollEvents();
}

static void shiz_draw_path_3d(SHIZVector3 const points[], uint const count, SHIZColor const color) {
    SHIZVertexPositionColor vertices[count];

    for (uint i = 0; i < count; i++) {
        vertices[i].position = points[i];
        vertices[i].color = color;
    }

    shiz_gfx_render(GL_LINE_STRIP, vertices, count);
}

static void shiz_draw_line_3d(SHIZVector3 const from, SHIZVector3 const to, SHIZColor const color) {
    SHIZVector3 points[] = {
        from, to
    };

    shiz_draw_path_3d(points, 2, color);
}

void shiz_draw_line(SHIZVector2 const from, SHIZVector2 const to, SHIZColor const color) {
    shiz_draw_line_3d(SHIZVector3Make(from.x, from.y, 0),
                      SHIZVector3Make(to.x, to.y, 0),
                      color);
}

void shiz_draw_path(SHIZVector2 const points[], uint const count, SHIZColor const color) {
    SHIZVector3 points3[count];

    for (uint i = 0; i < count; i++) {
        points3[i].x = points[i].x;
        points3[i].y = points[i].y;
    }

    shiz_draw_path_3d(points3, count, color);
}

SHIZRect _shiz_get_anchored_rect(SHIZSize const size, SHIZVector2 const anchor) {
    float const hw = size.width / 2;
    float const hh = size.height / 2;
    
    float const dx = hw * -anchor.x;
    float const dy = hh * -anchor.y;
    
    float const l = dx - hw;
    float const b = dy - hh;
    
    return SHIZRectMake(SHIZVector2Make(l, b), size);
}

static void _shiz_draw_rect(SHIZRect const rect, SHIZColor const color, bool const fill, SHIZVector2 const anchor, float const angle) {
    uint const vertex_count = fill ? 4 : 5; // only drawing the shape requires an additional vertex

    SHIZVertexPositionColor vertices[vertex_count];

    for (uint i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
    }

    SHIZVector3 const origin = SHIZVector3Make(rect.origin.x, rect.origin.y, 0);

    SHIZRect const anchored_rect = _shiz_get_anchored_rect(rect.size, anchor);
    
    float const l = anchored_rect.origin.x;
    float const r = anchored_rect.origin.x + anchored_rect.size.width;
    float const b = anchored_rect.origin.y;
    float const t = anchored_rect.origin.y + anchored_rect.size.height;

    if (!fill) {
        vertices[0].position = SHIZVector3Make(l, b, 0);
        vertices[1].position = SHIZVector3Make(l, t, 0);
        // note that the order of the vertices differ from the filled shape
        vertices[2].position = SHIZVector3Make(r, t, 0);
        vertices[3].position = SHIZVector3Make(r, b, 0);
        // the additional vertex connects to the beginning, to complete the shape
        vertices[4].position = vertices[0].position;

        shiz_gfx_render_ex(GL_LINE_STRIP, vertices, vertex_count, origin, angle);
    } else {
        vertices[0].position = SHIZVector3Make(l, b, 0);
        vertices[1].position = SHIZVector3Make(l, t, 0);
        vertices[2].position = SHIZVector3Make(r, b, 0);
        vertices[3].position = SHIZVector3Make(r, t, 0);

        shiz_gfx_render_ex(GL_TRIANGLE_STRIP, vertices, vertex_count, origin, angle);
    }
}

void shiz_draw_rect(SHIZRect const rect, SHIZColor const color) {
    _shiz_draw_rect(rect, color, true, SHIZSpriteAnchorBottomLeft, 0);
}

void shiz_draw_rect_ex(SHIZRect const rect, SHIZColor const color, SHIZVector2 const anchor, float const angle) {
    _shiz_draw_rect(rect, color, true, anchor, angle);
}

void shiz_draw_rect_shape(SHIZRect const rect, SHIZColor const color) {
    _shiz_draw_rect(rect, color, false, SHIZSpriteAnchorBottomLeft, 0);
}

void shiz_draw_rect_shape_ex(SHIZRect const rect, SHIZColor const color, SHIZVector2 const anchor, float const angle) {
    _shiz_draw_rect(rect, color, false, anchor, angle);
}

void shiz_draw_sprite(SHIZSprite const sprite, SHIZVector2 const origin) {
    shiz_draw_sprite_ex(sprite, origin,
                        SHIZSpriteSizeIntrinsic,
                        SHIZSpriteAnchorCenter,
                        SHIZSpriteNoAngle,
                        SHIZSpriteNoTint,
                        SHIZSpriteNoRepeat,
                        false,
                        SHIZSpriteLayerDefault,
                        SHIZSpriteLayerDepthDefault);
}

void shiz_draw_sprite_ex(SHIZSprite const sprite, SHIZVector2 const origin, SHIZSize const size, SHIZVector2 const anchor, float const angle, SHIZColor const tint, bool const repeat, bool const opaque, unsigned char const layer, unsigned short const depth) {
    SHIZResourceImage image = shiz_res_get_image(sprite.resource_id);

    if (image.id == sprite.resource_id) {
        uint const vertex_count = 6;

        SHIZVertexPositionColorTexture vertices[vertex_count];

        for (uint i = 0; i < vertex_count; i++) {
            vertices[i].color = tint;
        }

        SHIZSize const working_size = (size.width == SHIZSpriteSizeIntrinsic.width &&
                                       size.height == SHIZSpriteSizeIntrinsic.height) ? sprite.source.size : size;

        SHIZRect const anchored_rect = _shiz_get_anchored_rect(working_size, anchor);
        
        float const l = anchored_rect.origin.x;
        float const r = anchored_rect.origin.x + anchored_rect.size.width;
        float const b = anchored_rect.origin.y;
        float const t = anchored_rect.origin.y + anchored_rect.size.height;
        
        SHIZVector2 bl = SHIZVector2Make(l, b);
        SHIZVector2 tl = SHIZVector2Make(l, t);
        SHIZVector2 tr = SHIZVector2Make(r, t);
        SHIZVector2 br = SHIZVector2Make(r, b);

        vertices[0].position = SHIZVector3Make(tl.x, tl.y, 0);
        vertices[1].position = SHIZVector3Make(br.x, br.y, 0);
        vertices[2].position = SHIZVector3Make(bl.x, bl.y, 0);

        vertices[3].position = SHIZVector3Make(tl.x, tl.y, 0);
        vertices[4].position = SHIZVector3Make(tr.x, tr.y, 0);
        vertices[5].position = SHIZVector3Make(br.x, br.y, 0);

        SHIZRect source = sprite.source;

        bool const flip_vertically = true;

        if (flip_vertically) {
            // opengl assumes that the origin of textures is at the bottom-left of the image,
            // however, it is common to specify top-left as origin when using e.g. sprite sheets (and we want that)
            // so, assuming that the provided source frame expects the top-left to be the origin,
            // we have to flip the specified coordinate so that the origin becomes bottom-left
            source.origin.y = (image.height - source.size.height) - source.origin.y;
        }

        SHIZVector2 const uv_min = SHIZVector2Make((source.origin.x / image.width),
                                                   (source.origin.y / image.height));
        SHIZVector2 const uv_max = SHIZVector2Make(((source.origin.x + source.size.width) / image.width),
                                                   ((source.origin.y + source.size.height) / image.height));

        float uv_scale_x = 1;
        float uv_scale_y = 1;

        if (repeat) {
            // in order to repeat a texture, we need to scale the uv's to be larger than the actual source
            if (working_size.width > sprite.source.size.width) {
                uv_scale_x = working_size.width / sprite.source.size.width;
            }

            if (working_size.height > sprite.source.size.height) {
                uv_scale_y = working_size.height / sprite.source.size.height;
            }
        }

        tl = SHIZVector2Make(uv_min.x * uv_scale_x, uv_max.y * uv_scale_y);
        br = SHIZVector2Make(uv_max.x * uv_scale_x, uv_min.y * uv_scale_y);
        bl = SHIZVector2Make(uv_min.x * uv_scale_x, uv_min.y * uv_scale_y);
        tr = SHIZVector2Make(uv_max.x * uv_scale_x, uv_max.y * uv_scale_y);

        vertices[0].texture_coord = tl;
        vertices[1].texture_coord = br;
        vertices[2].texture_coord = bl;

        vertices[3].texture_coord = tl;
        vertices[4].texture_coord = tr;
        vertices[5].texture_coord = br;

        for (uint i = 0; i < vertex_count; i++) {
            // in order for repeated textures to work (without having to set wrapping modes, and with support for sub-textures)
            // we have to specify the space that uv's are limited to (otherwise a sub-texture with a
            // scaled uv would just end up using part of another subtexture- we don't want that)
            // so this solution will simply "loop over" a scaled uv coordinate so that it is restricted
            // within the dimensions of the expected texture
            vertices[i].texture_coord_min = uv_min;
            vertices[i].texture_coord_max = uv_max;
        }

        // range within [-1;0], where -1 is nearest (so layer 128 should be z = -1)
        float const z = -((layer - _shiz_sprite_layer_min) / (_shiz_sprite_layer_max - _shiz_sprite_layer_min));

        unsigned long key = 0;

        SHIZSpriteInternalKey * sprite_key = (SHIZSpriteInternalKey *)&key;

        sprite_key->layer = layer;
        sprite_key->layer_depth = depth;
        sprite_key->texture_id = image.texture_id;
        sprite_key->is_transparent = !opaque;

        SHIZSpriteInternal * sprite_internal = &_shiz_sprites[_shiz_sprites_count];

        sprite_internal->key = key;
        sprite_internal->angle = angle;
        sprite_internal->origin = SHIZVector3Make(origin.x, origin.y, z);

        for (uint i = 0; i < vertex_count; i++) {
            sprite_internal->vertices[i].position = vertices[i].position;
            sprite_internal->vertices[i].texture_coord = vertices[i].texture_coord;
            sprite_internal->vertices[i].texture_coord_max = vertices[i].texture_coord_max;
            sprite_internal->vertices[i].texture_coord_min = vertices[i].texture_coord_min;
            sprite_internal->vertices[i].color = vertices[i].color;
        }

        _shiz_sprites_count++;

        if (_shiz_sprites_count > SHIZSpriteInternalMax) {
            shiz_io_warning("sprite limit reached (%d); sprites may not draw as expected", SHIZSpriteInternalMax);

            _shiz_flush_sprites();
        }
    }
}

static int _shiz_compare_sprites(const void * a, const void * b) {
    SHIZSpriteInternal const * lhs = (SHIZSpriteInternal *)a;
    SHIZSpriteInternal const * rhs = (SHIZSpriteInternal *)b;

    if (lhs->key < rhs->key) {
        return -1;
    } else if (lhs->key > rhs->key) {
        return 1;
    }

    return 0;
}

static void _shiz_flush_sprites() {
    qsort(_shiz_sprites, _shiz_sprites_count, sizeof(SHIZSpriteInternal),
          _shiz_compare_sprites);

    for (uint sprite_index = 0; sprite_index < _shiz_sprites_count; sprite_index++) {
        SHIZSpriteInternal const sprite = _shiz_sprites[sprite_index];
        SHIZSpriteInternalKey * const sprite_key = (SHIZSpriteInternalKey *)&sprite.key;

        shiz_gfx_render_quad(sprite.vertices, sprite.origin, sprite.angle, sprite_key->texture_id);
    }

    _shiz_sprites_count = 0;
}

SHIZSize shiz_measure_sprite_text(SHIZSpriteFont const font, const char * const text, SHIZSize const bounds, SHIZSpriteFontAttributes const attributes) {
    SHIZSpriteFontMeasurement measurement = _shiz_measure_sprite_text(font, text, bounds, attributes);

    return measurement.size;
}

static SHIZSpriteFontMeasurement _shiz_measure_sprite_text(SHIZSpriteFont const font, const char * const text, SHIZSize const bounds, SHIZSpriteFontAttributes const attributes) {
    SHIZSpriteFontMeasurement measurement;

    measurement.size = SHIZSizeEmpty;
    measurement.constrain_index = -1; // no truncation

    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    measurement.character_size = SHIZSizeMake(character_sprite.source.size.width * attributes.scale.x,
                                              character_sprite.source.size.height * attributes.scale.y);

    measurement.character_size_perceived = SHIZSizeMake((measurement.character_size.width * attributes.character_spread) + attributes.character_padding,
                                                        measurement.character_size.height);

    measurement.constrain_horizontally = bounds.width != SHIZSpriteFontSizeToFit.width;
    measurement.constrain_vertically = bounds.height != SHIZSpriteFontSizeToFit.height;

    measurement.max_characters_per_line = floor(bounds.width / measurement.character_size_perceived.width);
    measurement.max_lines_in_bounds = floor(bounds.height / measurement.character_size_perceived.height);

    float const line_height = measurement.character_size_perceived.height + attributes.line_padding;

    uint text_index = 0;
    uint line_index = 0;
    uint line_character_count = 0;
    uint line_character_ignored_count = 0;

    char const whitespace_character = ' ';
    char const newline_character = '\n';

    const char * text_ptr = text;

    while (*text_ptr) {
        char character = *text_ptr;

        text_ptr += _shiz_get_char_size(character);

        bool const break_line_explicit = character == newline_character;
        bool const break_line_required = (measurement.constrain_horizontally &&
                                          line_character_count >= measurement.max_characters_per_line);

        if (break_line_explicit || break_line_required) {
            if (break_line_required && attributes.wrap == SHIZSpriteFontWrapModeWord) {
                // backtrack until finding a whitespace
                while (*text_ptr) {
                    text_ptr -= _shiz_get_char_size(character);
                    text_index -= 1;

                    character = *text_ptr;

                    if (*text_ptr == whitespace_character) {
                        break;
                    }

                    line_character_count -= 1;
                }
            }

            measurement.lines[line_index].size.width = line_character_count * measurement.character_size_perceived.width;
            measurement.lines[line_index].size.height = line_height;
            measurement.lines[line_index].ignored_character_count = line_character_ignored_count;

            line_character_ignored_count = 0;
            line_character_count = 0;
            line_index += 1;

            if (line_index > SHIZSpriteFontMaxLines) {
                // this is bad
                break;
            }

            continue;
        }

        if (measurement.constrain_vertically) {
            if (line_index + 1 > measurement.max_lines_in_bounds) {
                measurement.constrain_index = text_index - 1; // it was actually the previous character that caused a linebreak

                break;
            }
        }

        if (character == '\1' || character == '\2' || character == '\3' || character == '\4' ||
            character == '\5' || character == '\6' || character == '\7') {
            // increment ignored characters, but otherwise proceed as usual
            line_character_ignored_count += 1;
        }

        // leave a space even if the character was not found
        line_character_count += 1;

        measurement.lines[line_index].size.width = line_character_count * measurement.character_size_perceived.width;
        measurement.lines[line_index].size.height = line_height;
        measurement.lines[line_index].ignored_character_count = line_character_ignored_count;

        text_index += 1;
    }

    measurement.line_count = line_index + 1;
    measurement.size.height = measurement.line_count * line_height;

    for (line_index = 0; line_index < measurement.line_count; line_index++) {
        SHIZSpriteFontLine const line = measurement.lines[line_index];

        if (line.size.width > measurement.size.width) {
            // use the widest occurring line width
            measurement.size.width = line.size.width;
        }
    }

    return measurement;
}

SHIZSize shiz_draw_sprite_text(SHIZSpriteFont const font, const char * const text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment) {
    return shiz_draw_sprite_text_ex(font, text, origin, alignment,
                                    SHIZSpriteFontSizeToFit,
                                    SHIZSpriteNoTint,
                                    SHIZSpriteFontAttributesDefault);
}

SHIZSize shiz_draw_sprite_text_ex(SHIZSpriteFont const font, const char * const text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment, SHIZSize const bounds, SHIZColor const tint, SHIZSpriteFontAttributes const attributes) {
    return shiz_draw_sprite_text_ex_colored(font, text, origin, alignment, bounds, tint, attributes, NULL, 0);
}

SHIZSize shiz_draw_sprite_text_ex_colored(SHIZSpriteFont const font, const char * const text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment, SHIZSize const bounds, SHIZColor const tint, SHIZSpriteFontAttributes const attributes, SHIZColor * const highlight_colors, uint const highlight_color_count) {
    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    SHIZSpriteFontMeasurement const measurement = _shiz_measure_sprite_text(font, text, bounds, attributes);

    uint const truncation_length = 3;
    char const truncation_character = '.';
    char const whitespace_character = ' ';
    char const newline_character = '\n';

    SHIZVector2 character_origin = origin;

    if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
        // intenionally left blank; no operation necessary
    } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
        character_origin.y += measurement.size.height / 2;
    } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
        character_origin.y += measurement.size.height;
    }

    uint text_index = 0;

    bool should_break_from_truncation = false;

    SHIZColor highlight_color = tint;

    for (uint line_index = 0; line_index < measurement.line_count; line_index++) {
        SHIZSpriteFontLine const line = measurement.lines[line_index];

        float const line_width_perceived = line.size.width - (line.ignored_character_count * measurement.character_size_perceived.width);

        if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
            character_origin.x -= line_width_perceived / 2;
        } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
            character_origin.x -= line_width_perceived;
        }

        uint const line_character_count = line.size.width / measurement.character_size_perceived.width;

        for (uint character_index = 0; character_index < line_character_count; character_index++) {
            bool const should_truncate = measurement.constrain_index != -1 && (text_index > measurement.constrain_index - truncation_length);

            should_break_from_truncation = measurement.constrain_index == text_index;

            char character = should_truncate ? truncation_character : text[text_index];

            text_index += 1;

            if (character == '\1' || character == '\2' || character == '\3' || character == '\4' ||
                character == '\5' || character == '\6' || character == '\7') {
                // these characters are only used for tinting purposes and will be ignored/skipped otherwise
                if (highlight_colors && highlight_color_count > 0) {
                    int const highlight_color_index = character - 2;

                    if (highlight_color_index < 0) {
                        highlight_color = tint;
                    } else {
                        if (highlight_color_index < highlight_color_count) {
                            highlight_color = highlight_colors[highlight_color_index];
                        }
                    }
                }

                continue;
            }

            if (character == newline_character) {
                // ignore newlines and just proceed as if this iteration never happened
                character_index--;

                continue;
            }

            int character_table_index = character - font.table.offset;

            if (character_table_index < 0 ||
                character_table_index > font.table.columns * font.table.rows) {
                character_table_index = -1;
            }

            bool const should_skip_leading_whitespace = !font.includes_whitespace && attributes.wrap == SHIZSpriteFontWrapModeWord;
            bool const is_leading_whitespace = character_index == 0 && character == whitespace_character;

            bool character_takes_space = true;

            if (is_leading_whitespace && should_skip_leading_whitespace) {
                character_takes_space = false;

                // the index has already been incremented once, so we have to step back by 2
                int const previous_text_index = text_index - 2;

                if (previous_text_index >= 0) {
                    char const previous_character = text[previous_text_index];

                    if (previous_character == newline_character) {
                        // this was an explicit line-break, so the leading whitespace is probably intentional
                        character_takes_space = true;
                    }
                }
            }

            if (character_table_index != -1) {
                bool can_draw_character = character != whitespace_character || font.includes_whitespace;

                if (can_draw_character) {
                    uint const character_row = (int)(character_table_index / (int)font.table.columns);
                    uint const character_column = character_table_index % (int)font.table.columns;

                    character_sprite.source.origin.x = font.sprite.source.origin.x + (font.character.width * character_column);
                    character_sprite.source.origin.y = font.sprite.source.origin.y + (font.character.height * character_row);

                    shiz_draw_sprite_ex(character_sprite, character_origin,
                                        measurement.character_size,
                                        SHIZSpriteAnchorTopLeft, SHIZSpriteNoAngle,
                                        highlight_color, SHIZSpriteNoRepeat, false,
                                        SHIZSpriteLayerDefault, SHIZSpriteLayerDepthDefault);
                }
            }

            if (character_takes_space) {
                character_origin.x += measurement.character_size_perceived.width;
            }

            if (should_break_from_truncation) {
                // we need to break out of everything once we reach the final visible character
                break;
            }
        }
        
        character_origin.x = origin.x;
        character_origin.y -= line.size.height;
        
        if (should_break_from_truncation) {
            break;
        }
    }
    
    return measurement.size;
}

#ifdef SHIZ_DEBUG
static void _shiz_debug_build_stats() {
    SHIZViewport const viewport = shiz_gfx_get_viewport();

    sprintf(_shiz_debug_font_buffer,
            "\4%.0fx%.0f\1@\5%.0fx%.0f\1\n\n"
            "\2%0.2fms/frame\1 (\4%0.2fms\1)\n"
            "\2%d\1 (\3%d\1|\4%d\1|\5%d\1)\n\n"
            "\2%d draws/frame\1",
            viewport.screen.width, viewport.screen.height,
            viewport.framebuffer.width, viewport.framebuffer.height,
            shiz_gfx_debug_get_frame_time(),
            shiz_gfx_debug_get_frame_time_avg(),
            shiz_gfx_debug_get_frames_per_second(),
            shiz_gfx_debug_get_frames_per_second_min(),
            shiz_gfx_debug_get_frames_per_second_avg(),
            shiz_gfx_debug_get_frames_per_second_max(),
            // note that draw count will also include the debug stuff, so in production
            // this count may actually be smaller (likely not significantly smaller, though)
            shiz_gfx_debug_get_draw_count());
}

static void _shiz_debug_display_stats() {
    uint const margin = 4;

    SHIZColor highlight_colors[] = {
        SHIZColorFromHex(0xefec0d), // yellow
        SHIZColorFromHex(0xe5152d), // red
        SHIZColorFromHex(0x36cd33), // green
        SHIZColorFromHex(0x20b1fc) // blue
    };

    shiz_draw_sprite_text_ex_colored(shiz_debug_font,
                                     _shiz_debug_font_buffer,
                                     SHIZVector2Make(shiz_context.preferred_screen_size.width - margin,
                                                     shiz_context.preferred_screen_size.height - margin),
                                     SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentRight,
                                     SHIZSpriteFontSizeToFit, SHIZSpriteNoTint, SHIZSpriteFontAttributesDefault,
                                     highlight_colors, 4);
}

static void _shiz_debug_process_errors() {
    GLenum error;

    while ((error = glGetError()) != GL_NO_ERROR) {
        shiz_io_error_context("OPENGL", "%d", error);
    }
}
#endif
