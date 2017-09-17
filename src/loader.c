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

#include <SHIZEN/loader.h>

#include "res.h"

unsigned int
z_load(char const * const filename)
{
    return z_res__load(z_res__type(filename), filename);
}

bool
z_unload(unsigned int const resource_id)
{
    return z_res__unload(resource_id);
}


SHIZSprite
z_load_sprite(char const * const filename)
{
    unsigned int const resource_id = z_load(filename);
    
    if (resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    return z_load_sprite_from(resource_id);
}

SHIZSprite
z_load_sprite_src(char const * const filename,
                  SHIZRect const source)
{
    SHIZSprite const sprite = z_load_sprite(filename);
    
    if (sprite.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    return z_load_sprite_from_src(sprite.resource_id, source);
}

SHIZSpriteSheet
z_load_spritesheet(char const * const filename,
                   SHIZSize const sprite_size)
{
    SHIZSprite const sprite = z_load_sprite(filename);
    
    if (sprite.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteSheetEmpty;
    }
    
    return z_load_spritesheet_from(sprite, sprite_size);
}

SHIZSpriteSheet
z_load_spritesheet_src(char const * const filename,
                       SHIZSize const sprite_size,
                       SHIZRect const source)
{
    SHIZSpriteSheet const sprite_sheet =
        z_load_spritesheet(filename, sprite_size);
    
    if (sprite_sheet.resource.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteSheetEmpty;
    }
    
    return z_load_spritesheet_from_src(sprite_sheet.resource,
                                       sprite_size,
                                       source);
}

SHIZSprite
z_load_sprite_from(unsigned int const resource_id)
{
    SHIZResourceImage const image = z_res__image(resource_id);
    
    if (image.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    SHIZRect const source = SHIZRectMake(SHIZVector2Zero,
                                         SHIZSizeMake(image.width, image.height));
    
    return z_load_sprite_from_src(resource_id, source);
}

SHIZSprite
z_load_sprite_from_src(unsigned int const resource_id,
                       SHIZRect const source)
{
    SHIZSprite sprite;
    
    sprite.resource_id = resource_id;
    sprite.source = source;
    
    return sprite;
}

SHIZSpriteSheet
z_load_spritesheet_from(SHIZSprite const resource,
                        SHIZSize const sprite_size)
{
    SHIZSpriteSheet spritesheet;
    
    spritesheet.resource = resource;
    spritesheet.sprite_size = sprite_size;
    
    SHIZSize const source = resource.source.size;
    
    if (source.width > sprite_size.width) {
        spritesheet.columns = (unsigned int)(source.width / sprite_size.width);
    } else {
        spritesheet.columns = 1;
    }
    
    if (source.height > sprite_size.height) {
        spritesheet.rows = (unsigned int)(source.height / sprite_size.height);
    } else {
        spritesheet.rows = 1;
    }
    
    spritesheet.sprite_padding = SHIZSizeZero;
    
    return spritesheet;
}

SHIZSpriteSheet
z_load_spritesheet_from_src(SHIZSprite const resource,
                            SHIZSize const sprite_size,
                            SHIZRect const source)
{
    SHIZSprite const sprite = z_load_sprite_from_src(resource.resource_id,
                                                     source);
    
    return z_load_spritesheet_from(sprite, sprite_size);
}

SHIZSprite
z_load_sprite_from_index(SHIZSpriteSheet const spritesheet,
                         unsigned int const index)
{
    unsigned int const row = (unsigned int)(index / spritesheet.columns);
    unsigned int const column = index % spritesheet.columns;
    
    SHIZVector2 const source_origin = SHIZVector2Make(spritesheet.resource.source.origin.x +
                                                      spritesheet.sprite_padding.width,
                                                      spritesheet.resource.source.origin.y +
                                                      spritesheet.sprite_padding.height);
    
    SHIZVector2 const origin = SHIZVector2Make(source_origin.x +
                                               (column * spritesheet.sprite_size.width),
                                               source_origin.y +
                                               (row * spritesheet.sprite_size.height));
    
    SHIZSize const size = SHIZSizeMake(spritesheet.sprite_size.width -
                                       (spritesheet.sprite_padding.width * 2),
                                       spritesheet.sprite_size.height -
                                       (spritesheet.sprite_padding.height * 2));
    
    SHIZRect const sprite_frame = SHIZRectMake(origin, size);
    
    return z_load_sprite_from_src(spritesheet.resource.resource_id,
                                  sprite_frame);
}

SHIZSprite
z_load_sprite_from_cell(SHIZSpriteSheet const spritesheet,
                        unsigned int const column,
                        unsigned int const row)
{
    unsigned int const index = row * spritesheet.columns + column;
    
    return z_load_sprite_from_index(spritesheet, index);
}

SHIZSpriteFont
z_load_spritefont(char const * const filename,
                  SHIZSize const character)
{
    SHIZSprite const sprite = z_load_sprite(filename);
    
    return z_load_spritefont_from(sprite, character);
}

SHIZSpriteFont
z_load_spritefont_ex(char const * const filename,
                     SHIZSize const character,
                     SHIZSpriteFontTable const table)
{
    SHIZSpriteFont const spritefont = z_load_spritefont(filename, character);
    
    return z_load_spritefont_from_ex(spritefont.sprite,
                                     spritefont.character,
                                     table);
}

SHIZSpriteFont
z_load_spritefont_from(SHIZSprite const sprite,
                       SHIZSize const character)
{
    SHIZSpriteFontTable table;
    
    SHIZSize const source = sprite.source.size;
    
    if (source.width > character.width) {
        table.columns = (unsigned int)(source.width / character.width);
    } else {
        table.columns = 1;
    }
    
    if (source.height > character.height) {
        table.rows = (unsigned int)(source.height / character.height);
    } else {
        table.rows = 1;
    }
    
    table.codepage = 0;
    
    return z_load_spritefont_from_ex(sprite, character, table);
}

SHIZSpriteFont
z_load_spritefont_from_ex(SHIZSprite const sprite,
                          SHIZSize const character,
                          SHIZSpriteFontTable const table)
{
    SHIZSpriteFont spritefont;
    
    spritefont.sprite = sprite;
    spritefont.character = character;
    spritefont.table = table;
    // default to skip whitespaces; this will reduce the number of sprites drawn
    spritefont.includes_whitespace = false;
    
    return spritefont;
}
