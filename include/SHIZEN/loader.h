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

#ifndef loader_h
#define loader_h

#include <stdbool.h>

#include "type.h"

/**
 * @brief Load a resource.
 *
 * @return A resource id if the resource was loaded successfully, `0` otherwise
 */
unsigned int shiz_load(char const * filename);
/**
 * @brief Unload a resource.
 *
 * @return `true` if the resource was unloaded successfully, `false` otherwise
 */
bool shiz_unload(unsigned int resource_id);

SHIZSprite shiz_load_sprite(char const * filename);
SHIZSprite shiz_load_sprite_src(char const * filename,
                                SHIZRect source);

SHIZSprite shiz_get_sprite(unsigned int resource_id);
SHIZSprite shiz_get_sprite_src(unsigned int resource_id,
                               SHIZRect source);

SHIZSpriteSheet shiz_load_sprite_sheet(char const * filename,
                                       SHIZSize sprite_size);
SHIZSpriteSheet shiz_load_sprite_sheet_src(char const * filename,
                                           SHIZSize sprite_size,
                                           SHIZRect source);

SHIZSpriteSheet shiz_get_sprite_sheet(SHIZSprite resource,
                                      SHIZSize sprite_size);
SHIZSpriteSheet shiz_get_sprite_sheet_src(SHIZSprite resource,
                                          SHIZSize sprite_size,
                                          SHIZRect source);

SHIZSprite shiz_get_sprite_index(SHIZSpriteSheet spritesheet,
                                 unsigned int index);
SHIZSprite shiz_get_sprite_colrow(SHIZSpriteSheet spritesheet,
                                  unsigned int column,
                                  unsigned int row);

SHIZSpriteFont shiz_load_sprite_font(char const * filename,
                                     SHIZSize character);
SHIZSpriteFont shiz_load_sprite_font_ex(char const * filename,
                                        SHIZSize character,
                                        SHIZSpriteFontTable table);

SHIZSpriteFont shiz_get_sprite_font(SHIZSprite sprite,
                                    SHIZSize character);
SHIZSpriteFont shiz_get_sprite_font_ex(SHIZSprite sprite,
                                       SHIZSize character,
                                       SHIZSpriteFontTable table);

#endif /* loader_h */
