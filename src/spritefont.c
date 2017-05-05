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

#include "spritefont.h"

#include "sprite.h"

#include <math.h>

static unsigned int
utf8_decode(const char * str, unsigned int * i)
{
    const unsigned char *s = (const unsigned char *)str;

    int u = *s, l = 1;

    if (((u) & 0xc0) == 0xc0) {
        int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;

        if (a < 6 || !(u & 0x02)) {
            int b = 0;

            u = ((u << (a + 1)) & 0xff) >> (a + 1);

            for (b = 1; b < a; ++b) {
                u = (u << 6) | (s[l++] & 0x3f);
            }
        }
    }

    if (i) {
        *i = l;
    }

    return u;
}

SHIZSpriteFontMeasurement const
shiz_sprite_measure_text(SHIZSpriteFont const font,
                         const char * const text,
                         SHIZSize const bounds,
                         SHIZSpriteFontAttributes const attributes)
{
    SHIZSpriteFontMeasurement measurement;

    measurement.size = SHIZSizeZero;
    measurement.max_characters = -1; // no truncation

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

    unsigned int character_count = 0;
    unsigned int line_index = 0;
    unsigned int line_character_count = 0;
    unsigned int line_character_ignored_count = 0;

    char const whitespace_character = ' ';
    char const newline_character = '\n';

    bool const skip_leading_whitespace = (attributes.wrap == SHIZSpriteFontWrapModeWord &&
                                          !font.includes_whitespace);

    bool current_line_has_leading_whitespace = false;
    bool next_line_has_leading_whitespace = false;

    const char * text_ptr = text;

    while (*text_ptr) {
        char character = *text_ptr;
        unsigned int character_size = 0;

        utf8_decode(text_ptr, &character_size);

        text_ptr += character_size;

        bool const break_line_explicit = character == newline_character;
        bool const break_line_required = (measurement.constrain_horizontally &&
                                          line_character_count >= measurement.max_characters_per_line);

        if (break_line_explicit || break_line_required) {
            next_line_has_leading_whitespace = false;

            if (break_line_required && attributes.wrap == SHIZSpriteFontWrapModeWord) {
                // backtrack until finding a whitespace
                while (*text_ptr) {
                    text_ptr -= character_size;
                    
                    character_count -= 1;

                    utf8_decode(text_ptr, &character_size);

                    character = *text_ptr;

                    if (character == whitespace_character) {
                        next_line_has_leading_whitespace = true;

                        break;
                    }

                    if (line_character_count > 0) {
                        line_character_count -= 1;
                    } else {
                        break;
                    }
                    
                    if (character_count == 0) {
                        // additional safety measure
                        break;
                    }
                }
            }

            unsigned int character_count_perceived = line_character_count - line_character_ignored_count;

            if (skip_leading_whitespace && current_line_has_leading_whitespace) {
                character_count_perceived -= 1;
            }

            measurement.lines[line_index].size.width = character_count_perceived * measurement.character_size_perceived.width;
            measurement.lines[line_index].size.height = line_height;
            measurement.lines[line_index].character_count = line_character_count;

            line_character_ignored_count = 0;
            line_character_count = 0;
            line_index += 1;

            current_line_has_leading_whitespace = next_line_has_leading_whitespace;

            if (line_index > SHIZSpriteFontMaxLines) {
                // this is bad
                break;
            }

            continue;
        }

        if (measurement.constrain_vertically) {
            if (line_index + 1 > measurement.max_lines_in_bounds) {
                // it was actually the previous character that caused a linebreak
                measurement.max_characters = character_count - 1;

                break;
            }
        }

        if (character == '\1' || character == '\2' || character == '\3' || character == '\4' ||
            character == '\5' || character == '\6' || character == '\7') {
            // increment ignored characters, but otherwise proceed as usual
            line_character_ignored_count += 1;
        }

        line_character_count += 1;
        character_count += 1;
        
        unsigned int character_count_perceived = line_character_count - line_character_ignored_count;

        if (skip_leading_whitespace && current_line_has_leading_whitespace) {
            character_count_perceived -= 1;
        }

        measurement.lines[line_index].size.width = character_count_perceived * measurement.character_size_perceived.width;
        measurement.lines[line_index].size.height = line_height;
        measurement.lines[line_index].character_count = line_character_count;
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

SHIZSize const
shiz_sprite_draw_text(SHIZSpriteFont const font,
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
    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    SHIZSpriteFontMeasurement const measurement = shiz_sprite_measure_text(font, text, bounds,
                                                                           attributes);

    unsigned int const truncation_length = 3;
    
    char const truncation_character = '.';
    char const whitespace_character = ' ';
    char const newline_character = '\n';

    SHIZVector2 character_origin = origin;

    if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
        // intentionally left blank; no operation necessary
    } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
        character_origin.y += measurement.size.height / 2;
    } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
        character_origin.y += measurement.size.height;
    }

    unsigned int character_count = 0;

    bool break_from_truncation = false;

    SHIZColor highlight_color = tint;

    const char * text_ptr = text;

    for (unsigned int line_index = 0; line_index < measurement.line_count; line_index++) {
        SHIZSpriteFontLine const line = measurement.lines[line_index];

        if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
            character_origin.x -= line.size.width / 2;
        } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
            character_origin.x -= line.size.width;
        }

        int character_index;

        for (character_index = 0;
             character_index < (int)line.character_count;
             character_index++) {
            bool const should_truncate =
                (measurement.max_characters != -1 &&
                 character_count > (measurement.max_characters - truncation_length));

            break_from_truncation =
                measurement.max_characters != -1 &&
                (unsigned int)measurement.max_characters == character_count;

            char const character = should_truncate ?
                truncation_character : *text_ptr;

            unsigned int character_size = 0;
            // for special characters we need to find the character decimal value
            // so that we can later look up the proper index in the font codepage
            unsigned int character_decimal = utf8_decode(text_ptr, &character_size);

            text_ptr += character_size;
            
            character_count += 1;

            if (character == '\1' || character == '\2' || character == '\3' || character == '\4' ||
                character == '\5' || character == '\6' || character == '\7') {
                // these characters are only used for tinting purposes and will be ignored/skipped otherwise
                if (highlight_colors && highlight_color_count > 0) {
                    // at this point, we know that 'character' is one of the numeric tint specifiers
                    // so we can determine the index like below, where a tint specifier of 1 results
                    // in an index of -1, which we then use to reset any highlight
                    int const highlight_color_index = character - 2;

                    if (highlight_color_index < 0) {
                        // reset to original tint
                        highlight_color = tint;
                    } else {
                        if (highlight_color_index < (int)highlight_color_count) {
                            highlight_color = highlight_colors[highlight_color_index];
                        }
                    }
                }

                continue;
            }

            if (character == newline_character) {
                // ignore newlines and just proceed as if this iteration never happened
                // note that this may cause character_index to rollover
                // but this will just cause the loop to end, which is intended
                character_index--;

                continue;
            }

            unsigned int const table_size = font.table.columns * font.table.rows;

            int character_table_index = -1;
            
            if (font.table.codepage != 0) {
                for (unsigned int i = 0; i < table_size; i++) {
                    if (font.table.codepage[i] == character_decimal) {
                        character_table_index = i;
                        
                        break;
                    }
                }
            } else {
                character_table_index = (unsigned char)character;
            }

            if (character_table_index < 0 ||
                character_table_index > (int)table_size) {
                character_table_index = -1;
            }

            bool const skip_leading_whitespace = (attributes.wrap == SHIZSpriteFontWrapModeWord &&
                                                  !font.includes_whitespace);
            bool const is_leading_whitespace = (character == whitespace_character &&
                                                character_index == 0);

            bool character_takes_space = true;

            if (is_leading_whitespace && skip_leading_whitespace) {
                // this character probably shouldn't take up any space;
                // however, in some cases, a leading whitespace is intentional,
                // so we try to determine that by stepping backwards:
                // the index has already been incremented once, so we have to step back by 2

                character_takes_space = false;

                int const previous_text_index = character_count - 2;

                if (previous_text_index >= 0) {
                    // note that we don't care about character byte sizes here;
                    // we might stumble into the back-end of a larger character
                    // by stepping back like this, but it doesn't matter, as we're
                    // only interested in knowing whether it's a newline or not
                    char const previous_character = text[previous_text_index];

                    if (previous_character == newline_character) {
                        // the previous character was an explicit line-break,
                        // so the leading whitespace is probably intentional
                        // and should appear
                        character_takes_space = true;
                    }
                }
            }

            if (character_table_index != -1) {
                bool can_draw_character = (character != whitespace_character ||
                                           font.includes_whitespace);

                if (can_draw_character) {
                    unsigned int const character_row = (int)(character_table_index / font.table.columns);
                    unsigned int const character_column = character_table_index % font.table.columns;

                    character_sprite.source.origin.x = (font.sprite.source.origin.x +
                                                        (font.character.width * character_column));
                    character_sprite.source.origin.y = (font.sprite.source.origin.y +
                                                        (font.character.height * character_row));

                    shiz_sprite_draw(character_sprite, character_origin,
                                     SHIZSpriteSized(measurement.character_size, SHIZSpriteNoScale),
                                     SHIZAnchorTopLeft, SHIZSpriteNoAngle,
                                     highlight_color, SHIZSpriteNoRepeat, false,
                                     layer);
                }
            }

            if (character_takes_space) {
                character_origin.x += measurement.character_size_perceived.width;
            }

            if (break_from_truncation) {
                // we need to break out of everything once we reach the final visible character
                break;
            }
        }
        
        character_origin.x = origin.x;
        character_origin.y -= line.size.height;
        
        if (break_from_truncation) {
            break;
        }
    }
    
    return measurement.size;
}
