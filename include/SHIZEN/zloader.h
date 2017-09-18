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

#ifndef zloader_h
#define zloader_h

#include <stdbool.h>

#include "ztype.h"

/**
 * @brief Load a resource.
 *
 * @return A resource id if the resource was loaded successfully, `0` otherwise
 */
unsigned int z_load(char const * filename);
/**
 * @brief Unload a resource.
 *
 * @return `true` if the resource was unloaded successfully, `false` otherwise
 */
bool z_unload(unsigned int resource_id);

SHIZSprite z_load_sprite(char const * filename);
SHIZSprite z_load_sprite_src(char const * filename,
                             SHIZRect source);

SHIZSprite z_load_sprite_from(unsigned int resource_id);
SHIZSprite z_load_sprite_from_src(unsigned int resource_id,
                                  SHIZRect source);

SHIZSpriteSheet z_load_spritesheet(char const * filename,
                                   SHIZSize sprite_size);
SHIZSpriteSheet z_load_spritesheet_src(char const * filename,
                                       SHIZSize sprite_size,
                                       SHIZRect source);

SHIZSpriteSheet z_load_spritesheet_from(SHIZSprite resource,
                                        SHIZSize sprite_size);
SHIZSpriteSheet z_load_spritesheet_from_src(SHIZSprite resource,
                                            SHIZSize sprite_size,
                                            SHIZRect source);

SHIZSprite z_load_sprite_from_index(SHIZSpriteSheet spritesheet,
                                    unsigned int index);
SHIZSprite z_load_sprite_from_cell(SHIZSpriteSheet spritesheet,
                                   unsigned int column,
                                   unsigned int row);

SHIZSpriteFont z_load_spritefont(char const * filename,
                                 SHIZSize character);
SHIZSpriteFont z_load_spritefont_ex(char const * filename,
                                    SHIZSize character,
                                    SHIZSpriteFontTable table);

SHIZSpriteFont z_load_spritefont_from(SHIZSprite sprite,
                                      SHIZSize character);
SHIZSpriteFont z_load_spritefont_from_ex(SHIZSprite sprite,
                                         SHIZSize character,
                                         SHIZSpriteFontTable table);

#endif /* zloader_h */
