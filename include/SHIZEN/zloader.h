#pragma once

#include <stdbool.h> // bool
#include <stdint.h> // uint8_t, uint16_t, uint32_t

#include "ztype.h" // SHIZSprite, SHIZSpriteSheet, SHIZSpriteFont

/**
 * @brief Load a resource.
 *
 * @return A resource id if the resource was loaded successfully, `0` otherwise
 */
uint8_t z_load(char const * filename);

/**
 * @brief Unload a resource.
 *
 * @return `true` if the resource was unloaded successfully, `false` otherwise
 */
bool z_unload(uint8_t resource_id);

SHIZSprite z_load_sprite(char const * filename);
SHIZSprite z_load_sprite_src(char const * filename, SHIZRect source);
SHIZSprite z_load_sprite_from(uint8_t resource_id);
SHIZSprite z_load_sprite_from_src(uint8_t resource_id, SHIZRect source);

SHIZSpriteSheet z_load_spritesheet(char const * filename, SHIZSize sprite_size);
SHIZSpriteSheet z_load_spritesheet_src(char const * filename, SHIZSize sprite_size, SHIZRect source);
SHIZSpriteSheet z_load_spritesheet_from(SHIZSprite resource, SHIZSize sprite_size);
SHIZSpriteSheet z_load_spritesheet_from_src(SHIZSprite resource, SHIZSize sprite_size, SHIZRect source);

SHIZSprite z_load_sprite_from_index(SHIZSpriteSheet spritesheet, uint32_t index);
SHIZSprite z_load_sprite_from_cell(SHIZSpriteSheet spritesheet, uint16_t column, uint16_t row);
SHIZSprite z_load_sprite_from_point(SHIZSpriteSheet spritesheet, uint16_t x, uint16_t y);

SHIZSpriteFont z_load_spritefont(char const * filename, SHIZSize character_size);
SHIZSpriteFont z_load_spritefont_ex(char const * filename, SHIZSize character_size, SHIZSpriteFontTable);
SHIZSpriteFont z_load_spritefont_from(SHIZSprite sprite, SHIZSize character_size);
SHIZSpriteFont z_load_spritefont_from_ex(SHIZSprite sprite, SHIZSize character_size, SHIZSpriteFontTable);
