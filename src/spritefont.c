#include "spritefont.h"
#include "sprite.h"

#include <stdlib.h> // NULL
#include <math.h> // floorf

static
uint32_t
utf8_decode(char const * str, uint32_t * i)
{
    unsigned char const * s = (unsigned char const *)str;

    uint32_t u = *s, l = 1;

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

static
bool
z_spritefont__is_tint_character(char);

static
uint16_t
z_spritefont__perceived_count(uint16_t count,
                              uint16_t ignored_count,
                              bool skipping_leading_whitespace);

static
void
z_spritefont__set_line(SHIZSpriteFontLine * line,
                       SHIZSpriteFontMeasurement const * measurement,
                       float line_height,
                       uint16_t character_count,
                       uint16_t character_ignored_count,
                       bool skipping_leading_whitespace);

static
int32_t
z_spritefont__character_table_index(SHIZSpriteFont const * font,
                                    char character,
                                    uint32_t character_decimal);

static
void
z_spritefont__draw_character_index(SHIZSpriteFont const * font,
                                   SHIZSpriteFontMeasurement const * measurement,
                                   SHIZVector2 character_origin,
                                   uint16_t character_table_index,
                                   SHIZColor highlight_color,
                                   SHIZLayer layer);

SHIZSpriteFontMeasurement const
z_spritefont__measure_text(SHIZSpriteFont const font,
                           char const * const text,
                           SHIZSize const bounds,
                           SHIZSpriteFontAttributes const attribs)
{
    SHIZSpriteFontMeasurement measurement;

    measurement.size = SHIZSizeZero;
    measurement.max_characters = -1; // no truncation
    measurement.line_count = 0;

    measurement.character_size = SHIZSizeMake(font.character.width * attribs.scale.x,
                                              font.character.height * attribs.scale.y);
    
    measurement.character_size_perceived =
        SHIZSizeMake((measurement.character_size.width * attribs.character_spread) + attribs.character_padding,
                     measurement.character_size.height);

    measurement.constrain_horizontally = bounds.width > 0;
    measurement.constrain_vertically = bounds.height > 0;

    if (measurement.constrain_horizontally) {
        float const max_characters_per_line =
            floorf(bounds.width / measurement.character_size_perceived.width);
        
        if (max_characters_per_line > UINT16_MAX) {
            measurement.max_characters_per_line = UINT16_MAX;
        } else {
            measurement.max_characters_per_line = (uint16_t)max_characters_per_line;
        }
    } else {
        measurement.max_characters_per_line = UINT16_MAX;
    }

    float const line_height =
        measurement.character_size_perceived.height + attribs.line_padding;
    
    if (measurement.constrain_vertically) {
        float const max_lines_in_bounds =
            floorf(bounds.height / line_height);
        
        if (max_lines_in_bounds > SHIZSpriteFontMaxLines) {
            measurement.max_lines_in_bounds = SHIZSpriteFontMaxLines;
        } else {
            measurement.max_lines_in_bounds =
                (uint16_t)max_lines_in_bounds;
        }
    } else {
        measurement.max_lines_in_bounds = UINT16_MAX;
    }

    uint16_t character_count = 0;
    uint8_t line_index = 0;
    uint16_t line_character_count = 0;
    uint16_t line_character_ignored_count = 0;

    char const whitespace_character = ' ';
    char const newline_character = '\n';

    bool const skip_leading_whitespace = !font.includes_whitespace;

    bool current_line_has_leading_whitespace = false;
    bool next_line_has_leading_whitespace = false;

    char const * text_ptr = text;

    if (!text_ptr) {
        return measurement;
    }

    while (*text_ptr) {
        char character = *text_ptr;
        
        uint32_t character_size = 0;

        utf8_decode(text_ptr, &character_size);

        text_ptr += character_size;

        bool const break_line_explicit = character == newline_character;
        // don't skip leading whitespace on the first line or if intentionally breaking
        bool const can_skip_leading_whitespace = (line_index > 0 &&
                                                  !break_line_explicit);
        
        bool should_skip_leading_whitespace = can_skip_leading_whitespace &&
            (skip_leading_whitespace &&
             current_line_has_leading_whitespace);

        uint16_t const line_character_count_perceived =
            z_spritefont__perceived_count(line_character_count,
                                          line_character_ignored_count,
                                          should_skip_leading_whitespace);

        bool const break_line_required =
            (measurement.constrain_horizontally &&
             line_character_count_perceived >= measurement.max_characters_per_line);

        if (break_line_explicit || break_line_required) {
            next_line_has_leading_whitespace = false;

            if (break_line_required) {
                if (attribs.wrap == SHIZSpriteFontWrapModeWord) {
                    // backtrack until finding a whitespace
                    while (*text_ptr) {
                        text_ptr -= character_size;
                        
                        character_count -= 1;

                        utf8_decode(text_ptr, &character_size);

                        character = *text_ptr;

                        if (character == whitespace_character &&
                            !break_line_explicit) {
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
                } else {
                    text_ptr -= character_size;
                    
                    char const breaking_character = *text_ptr;
                    
                    next_line_has_leading_whitespace =
                        breaking_character == whitespace_character;
                    
                    if (next_line_has_leading_whitespace) {
                        text_ptr -= character_size;
                    }
                }
            }
            
            z_spritefont__set_line(&measurement.lines[line_index],
                                   &measurement,
                                   line_height,
                                   line_character_count,
                                   line_character_ignored_count,
                                   should_skip_leading_whitespace);
            
            line_character_ignored_count = 0;
            line_character_count = 0;

            current_line_has_leading_whitespace = next_line_has_leading_whitespace;

            if (measurement.constrain_vertically) {
                if (line_index + 1 >= measurement.max_lines_in_bounds) {
                    measurement.max_characters = (int)character_count;

                    break;
                }
            }

            line_index += 1;

            if (line_index >= SHIZSpriteFontMaxLines) {
                line_index -= 1;
                // this is bad
                break;
            }

            continue;
        }

        if (z_spritefont__is_tint_character(character)) {
            // increment ignored characters, but otherwise proceed as usual
            line_character_ignored_count += 1;
        }

        line_character_count += 1;
        character_count += 1;
        
        z_spritefont__set_line(&measurement.lines[line_index],
                               &measurement,
                               line_height,
                               line_character_count,
                               line_character_ignored_count,
                               should_skip_leading_whitespace);
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
z_spritefont__draw_text(SHIZSpriteFont const font,
                        char const * const text,
                        SHIZVector2 const origin,
                        SHIZSpriteFontAlignment const alignment,
                        SHIZSize const bounds,
                        SHIZSpriteFontAttributes const attribs,
                        SHIZColor const tint,
                        SHIZLayer const layer)
{
    SHIZSpriteFontMeasurement const measurement =
        z_spritefont__measure_text(font, text, bounds, attribs);

    uint8_t const truncation_length = 3;
    int32_t const truncation_index = measurement.max_characters - truncation_length;
    
    char const truncation_character = '.';
    char const whitespace_character = ' ';
    char const newline_character = '\n';

    SHIZVector2 character_origin = origin;

    if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
        // intentionally left blank; no operation necessary
    } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
        character_origin.y += floorf(measurement.size.height / 2);
    } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
        character_origin.y += measurement.size.height;
    }

    uint16_t character_count = 0;

    bool break_from_truncation = false;

    SHIZColor highlight_color = tint;

    char const * text_ptr = text;
    
    for (uint8_t line_index = 0; line_index < measurement.line_count; line_index++) {
        SHIZSpriteFontLine const line = measurement.lines[line_index];

        if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
            character_origin.x -= floorf(line.size.width / 2);
        } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
            character_origin.x -= line.size.width;
        }

        uint16_t character_index = 0;
        
        for (character_index = 0;
             character_index < line.character_count;
             character_index++) {
            bool const should_truncate =
                (measurement.max_characters > 0 &&
                 character_count > truncation_index);

            break_from_truncation =
                measurement.max_characters > 0 &&
                measurement.max_characters == character_count;

            char const character = should_truncate ?
                truncation_character : *text_ptr;

            uint32_t character_size = should_truncate ?
                sizeof(character) : 0;
            // for special characters we need to find the character decimal value
            // so that we can later look up the proper index in the font codepage
            uint32_t character_decimal = should_truncate ?
                (uint32_t)character : utf8_decode(text_ptr, &character_size);

            text_ptr += character_size;
            
            character_count += 1;

            if (z_spritefont__is_tint_character(character)) {
                // this character is only used for tinting purposes and
                // will be ignored/skipped otherwise
                if (attribs.colors && attribs.colors_count > 0) {
                    // at this point, we know that 'character' is one of the numeric tint specifiers
                    // so we can determine the index like below, where a tint specifier of 1 results
                    // in an index of -1, which we then use to reset any highlight
                    int16_t const highlight_color_index = character - 2;

                    if (highlight_color_index < 0) {
                        // reset to original tint
                        highlight_color = tint;
                    } else {
                        if (highlight_color_index < attribs.colors_count) {
                            highlight_color = attribs.colors[highlight_color_index];
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

            int32_t const character_table_index =
                z_spritefont__character_table_index(&font,
                                                    character,
                                                    character_decimal);

            bool const skip_leading_whitespace = (!font.includes_whitespace &&
                                                  line_index > 0);
            bool const is_leading_whitespace = (character == whitespace_character &&
                                                character_index == 0);

            bool character_takes_space = true;

            if (is_leading_whitespace && skip_leading_whitespace) {
                // this character probably shouldn't take up any space;
                // however, in some cases, a leading whitespace is intentional,
                // so we try to determine that by stepping backwards:
                // the index has already been incremented once, so we have to step back by 2

                character_takes_space = false;

                int32_t const previous_text_index = character_count - 2;

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

            if (character_table_index >= 0) {
                bool const can_draw_character = (character != whitespace_character ||
                                                 font.includes_whitespace);

                if (can_draw_character) {
                    z_spritefont__draw_character_index(&font,
                                                       &measurement,
                                                       character_origin,
                                                       (uint16_t)character_table_index,
                                                       highlight_color,
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

static
void
z_spritefont__draw_character_index(SHIZSpriteFont const * const font,
                                   SHIZSpriteFontMeasurement const * const measurement,
                                   SHIZVector2 const character_origin,
                                   uint16_t const character_table_index,
                                   SHIZColor const highlight_color,
                                   SHIZLayer const layer)
{
    uint16_t const character_row = character_table_index / font->table.columns;
    uint16_t const character_column = character_table_index % font->table.columns;
    
    SHIZSprite character_sprite = SHIZSpriteEmpty;
    
    character_sprite.resource_id = font->sprite.resource_id;
    character_sprite.source = SHIZRectMake(font->sprite.source.origin,
                                           font->character);
    
    character_sprite.source.origin.x = (font->sprite.source.origin.x +
                                        (font->character.width * character_column));
    character_sprite.source.origin.y = (font->sprite.source.origin.y +
                                        (font->character.height * character_row));
    

    SHIZSpriteSize const character_sprite_size =
        SHIZSpriteSized(measurement->character_size,
                        SHIZVector2One);
    
    // todo: optimization; we already know the Z because the layer does not change per character
    // todo: optimization; we don't need to calculate anchored rect each time either
    z_sprite__draw(character_sprite,
                   character_origin,
                   character_sprite_size,
                   SHIZSpriteNoRepeat,
                   SHIZAnchorTopLeft,
                   SHIZSpriteFlipModeNone,
                   SHIZSpriteNoAngle,
                   highlight_color,
                   SHIZSpriteNotOpaque,
                   layer);
}

static
int32_t
z_spritefont__character_table_index(SHIZSpriteFont const * const font,
                                    char const character,
                                    uint32_t const character_decimal)
{
    uint32_t const table_size = font->table.columns * font->table.rows;
    
    int32_t character_table_index = -1;
    
    if (table_size < INT32_MAX) {
        if (font->table.codepage != NULL) {
            for (uint32_t i = 0; i < table_size; i++) {
                if (font->table.codepage[i] == character_decimal) {
                    character_table_index = (int)i;
                    
                    break;
                }
            }
        } else {
            character_table_index = (unsigned char)character;
        }
        
        if (character_table_index < 0 ||
            character_table_index > (int32_t)table_size) {
            character_table_index = -1;
        }
    }
    
    return character_table_index;
}

static
void
z_spritefont__set_line(SHIZSpriteFontLine * const line,
                       SHIZSpriteFontMeasurement const * const measurement,
                       float const line_height,
                       uint16_t const character_count,
                       uint16_t const character_ignored_count,
                       bool const skipping_leading_whitespace)
{
    uint16_t const line_character_count_perceived =
        z_spritefont__perceived_count(character_count,
                                      character_ignored_count,
                                      skipping_leading_whitespace);
    
    line->size.width = line_character_count_perceived * measurement->character_size_perceived.width;
    line->size.height = line_height;
    line->character_count = character_count;
}

static
bool
z_spritefont__is_tint_character(char const character)
{
    return (character == '\1' ||
            // \1 is used to reset back to original tint
            // remaining options used to access supplied colors,
            // where \2 == the color at index 0, \3 == index 1, and so on
            character == '\2' || character == '\3' || character == '\4' ||
            character == '\5' || character == '\6' || character == '\7');
}

static
uint16_t
z_spritefont__perceived_count(uint16_t const count,
                              uint16_t const ignored_count,
                              bool const skipping_leading_whitespace)
{
    uint16_t count_perceived = count;
    
    if (count_perceived >= ignored_count) {
        count_perceived -= ignored_count;
    } else {
        count_perceived = 0;
    }
    
    if (skipping_leading_whitespace) {
        if (count_perceived > 0) {
            count_perceived -= 1;
        }
    }
    
    return count_perceived;
}
