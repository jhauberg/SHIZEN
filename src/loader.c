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
shiz_load(char const * const filename)
{
    return shiz_res_load(shiz_res_get_type(filename), filename);
}

bool
shiz_unload(unsigned int const resource_id)
{
    return shiz_res_unload(resource_id);
}


SHIZSprite
shiz_load_sprite(char const * const filename)
{
    unsigned int const resource_id = shiz_load(filename);
    
    if (resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    return shiz_get_sprite(resource_id);
}

SHIZSprite
shiz_load_sprite_src(char const * const filename, SHIZRect const source)
{
    SHIZSprite const sprite = shiz_load_sprite(filename);
    
    if (sprite.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    return shiz_get_sprite_src(sprite.resource_id, source);
}

SHIZSpriteSheet
shiz_load_sprite_sheet(char const * const filename, SHIZSize const sprite_size)
{
    SHIZSprite const sprite = shiz_load_sprite(filename);
    
    if (sprite.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteSheetEmpty;
    }
    
    return shiz_get_sprite_sheet(sprite, sprite_size);
}

SHIZSpriteSheet
shiz_load_sprite_sheet_src(char const * const filename,
                           SHIZSize const sprite_size,
                           SHIZRect const source)
{
    SHIZSpriteSheet const sprite_sheet = shiz_load_sprite_sheet(filename, sprite_size);
    
    if (sprite_sheet.resource.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteSheetEmpty;
    }
    
    return shiz_get_sprite_sheet_src(sprite_sheet.resource, sprite_size, source);
}

SHIZSprite
shiz_get_sprite(unsigned int const resource_id)
{
    SHIZResourceImage const image = shiz_res_get_image(resource_id);
    
    if (image.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    SHIZRect const source = SHIZRectMake(SHIZVector2Zero,
                                         SHIZSizeMake(image.width, image.height));
    
    return shiz_get_sprite_src(resource_id, source);
}

SHIZSprite
shiz_get_sprite_src(unsigned int const resource_id, SHIZRect const source)
{
    SHIZSprite sprite;
    
    sprite.resource_id = resource_id;
    sprite.source = source;
    
    return sprite;
}

SHIZSpriteSheet
shiz_get_sprite_sheet(SHIZSprite const resource, SHIZSize const sprite_size)
{
    SHIZSpriteSheet spritesheet;
    
    spritesheet.resource = resource;
    spritesheet.sprite_size = sprite_size;
    
    spritesheet.columns = resource.source.size.width / sprite_size.width;
    spritesheet.rows = resource.source.size.height / sprite_size.height;
    spritesheet.sprite_padding = SHIZSizeZero;
    
    return spritesheet;
}

SHIZSpriteSheet
shiz_get_sprite_sheet_src(SHIZSprite const resource,
                          SHIZSize const sprite_size,
                          SHIZRect const source)
{
    return shiz_get_sprite_sheet(shiz_get_sprite_src(resource.resource_id, source), sprite_size);
}

SHIZSprite
shiz_get_sprite_index(SHIZSpriteSheet const spritesheet, unsigned int const index)
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
    
    return shiz_get_sprite_src(spritesheet.resource.resource_id, sprite_frame);
}

SHIZSprite
shiz_get_sprite_colrow(SHIZSpriteSheet const spritesheet,
                       unsigned int const column,
                       unsigned int const row)
{
    unsigned int const index = row * spritesheet.columns + column;
    
    return shiz_get_sprite_index(spritesheet, index);
}

SHIZSpriteFont
shiz_load_sprite_font(char const * const filename, SHIZSize const character)
{
    SHIZSprite const sprite = shiz_load_sprite(filename);
    
    return shiz_get_sprite_font(sprite, character);
}

SHIZSpriteFont
shiz_load_sprite_font_ex(char const * const filename,
                         SHIZSize const character,
                         SHIZSpriteFontTable const table)
{
    SHIZSpriteFont const spritefont = shiz_load_sprite_font(filename, character);
    
    return shiz_get_sprite_font_ex(spritefont.sprite, spritefont.character, table);
}

SHIZSpriteFont
shiz_get_sprite_font(SHIZSprite const sprite, SHIZSize const character)
{
    SHIZSpriteFontTable table;
    
    table.columns = sprite.source.size.width / character.width;
    table.rows = sprite.source.size.height / character.height;
    table.codepage = 0;
    
    return shiz_get_sprite_font_ex(sprite, character, table);
}

SHIZSpriteFont
shiz_get_sprite_font_ex(SHIZSprite const sprite,
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
