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

#include "sprite.h"

#include <stdlib.h> // qsort

#include "graphics/gfx.h"

#include "internal_type.h"
#include "res.h"

#ifdef SHIZ_DEBUG
 #include "debug/debug.h"
#endif

#define SHIZSpriteVertexCount 6

typedef struct SHIZSpriteObject {
    SHIZVertexPositionColorTexture vertices[SHIZSpriteVertexCount];
    SHIZVector3 origin;
    u32 key; // 4 bytes; will hold packed SHIZSpriteKey
    u32 order; // literal call order; used to ensure stable sorting
    f32 angle;
} SHIZSpriteObject;

typedef struct SHIZSpriteKey {
    // note that the order of these fields affect the sorting,
    // where fields at the bottom weigh heavier than ones at the top
    u8 texture_id; // 1 byte
    SHIZLayer layer; // 2 bytes; note that if we want more layers (as in >255, we need more bytes in our "container field", requiring a 64bit key i.e. u64)
    bool is_transparent; // 1 byte
} SHIZSpriteKey;

typedef struct SHIZSpriteList {
    SHIZSpriteObject sprites[SHIZSpriteMax];
    u32 total;
    u16 count;
    u8 _pad[2];
} SHIZSpriteList;

static
int
z_sprite__compare(void const * sprite,
                  void const * other_sprite);

static
void
z_sprite__sort(void);

static
void
z_sprite__set_position(SHIZSpriteObject * sprite,
                       SHIZSize destination_size,
                       SHIZVector2 anchor);
static
void
z_sprite__set_uv(SHIZSpriteObject * sprite,
                 SHIZSize destination_size,
                 SHIZSize texture_size,
                 SHIZRect source,
                 SHIZSpriteFlipMode flip,
                 SHIZColor tint,
                 bool repeat);

static struct SHIZSpriteList _sprite_list;

SHIZSize const
z_sprite__draw(SHIZSprite const sprite,
               SHIZVector2 const origin,
               SHIZSpriteSize const size,
               bool const repeat,
               SHIZVector2 const anchor,
               SHIZSpriteFlipMode flip,
               f32 const angle,
               SHIZColor const tint,
               bool const opaque,
               SHIZLayer const layer)
{
    SHIZResourceImage const image = z_res__image(sprite.resource_id);

    if (sprite.resource_id == SHIZResourceInvalid ||
        sprite.resource_id != image.resource_id ||
        (sprite.source.size.width <= 0 ||
         sprite.source.size.height <= 0)) {
        return SHIZSizeZero;
    }

    f32 const z = z_layer__get_z(layer);
    
    u32 sort_key = 0;
    
    SHIZSpriteKey * const sprite_key = (SHIZSpriteKey *)&sort_key;

    sprite_key->layer = layer;
    sprite_key->texture_id = (u8)image.texture_id;
    sprite_key->is_transparent = !opaque;
    
    struct SHIZSpriteObject * const sprite_object =
        &_sprite_list.sprites[_sprite_list.count];
    
    sprite_object->key = sort_key;
    sprite_object->angle = angle;
    sprite_object->order = _sprite_list.total;
    sprite_object->origin = SHIZVector3Make(roundf(origin.x),
                                            roundf(origin.y),
                                            z);

    SHIZSize const texture_size = SHIZSizeMake(image.width, image.height);

    SHIZSize const source_size = SHIZSizeMake(size.target.width > 0 ?
                                                size.target.width : sprite.source.size.width,
                                              size.target.height > 0 ?
                                                size.target.height : sprite.source.size.height);
    SHIZSize const destination_size = SHIZSizeMake(source_size.width * size.scale,
                                                   source_size.height * size.scale);

    // set vertex positions appropriately for the given anchor (note that vertices are not transformed until flushed)
    z_sprite__set_position(sprite_object, destination_size, anchor);
    // set texture coordinates appropriately, taking repeating/tiling into account
    z_sprite__set_uv(sprite_object, destination_size, texture_size,
                     sprite.source, flip, tint, repeat);

    // count for current batch
    _sprite_list.count += 1;
    // count for total sprites during a frame; i.e. the accumulation of all flushed sprites
    _sprite_list.total += 1;

    if (_sprite_list.count >= SHIZSpriteMax) {
        z_sprite__flush();
    }

    return destination_size;
}

SHIZRect const
z_sprite__anchor_rect(SHIZSize const size,
                      SHIZVector2 const anchor)
{
    f32 const hw = size.width / 2.0f;
    f32 const hh = size.height / 2.0f;

    f32 const dx = hw * -anchor.x;
    f32 const dy = hh * -anchor.y;

    f32 const l = dx - hw;
    f32 const b = dy - hh;

    return SHIZRectMake(SHIZVector2Make(l, b), size);
}

void
z_sprite__reset()
{
    _sprite_list.count = 0;
    _sprite_list.total = 0;
}

void
z_sprite__flush()
{
    if (_sprite_list.count == 0) {
        return;
    }
    
#ifdef SHIZ_DEBUG
    bool const should_print_order = z_debug__is_printing_sprite_order();
    
    if (should_print_order) {
        printf("-------- Z  LAYER ----- TEXTURE --------\n");
    }
#endif

    z_sprite__sort();
    
    for (u16 sprite_index = 0; sprite_index < _sprite_list.count; sprite_index++) {
        SHIZSpriteObject const sprite = _sprite_list.sprites[sprite_index];
        SHIZSpriteKey const * const sprite_key = (SHIZSpriteKey *)&sprite.key;

#ifdef SHIZ_DEBUG
        if (should_print_order) {
            printf("%.8f  [%03d,%05d] @%d (%s)\n",
                   sprite.origin.z,
                   sprite_key->layer.layer,
                   sprite_key->layer.depth,
                   sprite_key->texture_id,
                   sprite_key->is_transparent ? "transparent" : "opaque");
        }
#endif

        // finally push vertex data to the renderer
        z_gfx__render_sprite(sprite.vertices,
                             sprite.origin,
                             sprite.angle,
                             sprite_key->texture_id);
    }

    _sprite_list.count = 0;
}

static
int
z_sprite__compare(void const * const sprite,
                  void const * const other_sprite)
{
    SHIZSpriteObject const * lhs = (SHIZSpriteObject *)sprite;
    SHIZSpriteObject const * rhs = (SHIZSpriteObject *)other_sprite;
    
    if (lhs->key < rhs->key) {
        return -1;
    } else if (lhs->key > rhs->key) {
        return 1;
    } else if (lhs->order < rhs->order) {
        // fall back to using order of drawing if both keys are equal
        return -1;
    } else if (lhs->order > rhs->order) {
        return 1;
    }
    
    return 0;
}

static
void
z_sprite__sort()
{
    // sort sprites based on their layer parameters,
    // but also optimized for reduced state switching
    qsort(_sprite_list.sprites, _sprite_list.count,
          sizeof(SHIZSpriteObject),
          z_sprite__compare);
    
    // note that sorting can not guarantee correct order in cases where
    // flushing is required due to reaching sprite capacity
}

static
void
z_sprite__set_position(SHIZSpriteObject * const sprite,
                       SHIZSize const size,
                       SHIZVector2 const anchor)
{
    SHIZRect const anchored_rect = z_sprite__anchor_rect(size, anchor);
    
    f32 const l = anchored_rect.origin.x;
    f32 const r = anchored_rect.origin.x + anchored_rect.size.width;
    f32 const b = anchored_rect.origin.y;
    f32 const t = anchored_rect.origin.y + anchored_rect.size.height;
    
    SHIZVector2 const bl = SHIZVector2Make(l, b);
    SHIZVector2 const tl = SHIZVector2Make(l, t);
    SHIZVector2 const tr = SHIZVector2Make(r, t);
    SHIZVector2 const br = SHIZVector2Make(r, b);
    
    sprite->vertices[0].position = SHIZVector3Make(tl.x, tl.y, 0);
    sprite->vertices[1].position = SHIZVector3Make(br.x, br.y, 0);
    sprite->vertices[2].position = SHIZVector3Make(bl.x, bl.y, 0);
    
    sprite->vertices[3].position = SHIZVector3Make(tl.x, tl.y, 0);
    sprite->vertices[4].position = SHIZVector3Make(tr.x, tr.y, 0);
    sprite->vertices[5].position = SHIZVector3Make(br.x, br.y, 0);
}

static
void
z_sprite__set_uv(SHIZSpriteObject * const sprite,
                 SHIZSize const size,
                 SHIZSize const texture_size,
                 SHIZRect const source,
                 SHIZSpriteFlipMode const flip,
                 SHIZColor const tint,
                 bool const repeat)
{
    bool const flip_source_vertically = true;
    
    SHIZRect flipped_source = source;
    
    if (flip_source_vertically) {
        // opengl assumes that the origin of textures is at the bottom-left of the image,
        // however, it is common to specify top-left as origin when using e.g. sprite sheets (and we want that)
        // so, assuming that the provided source frame expects the top-left to be the origin,
        // we have to flip the specified coordinate so that the origin becomes bottom-left
        flipped_source.origin.y = (texture_size.height - source.size.height) - source.origin.y;
    }
    
    SHIZVector2 const uv_min =
        SHIZVector2Make(flipped_source.origin.x / texture_size.width,
                        flipped_source.origin.y / texture_size.height);
    SHIZVector2 const uv_max =
        SHIZVector2Make((flipped_source.origin.x + flipped_source.size.width) / texture_size.width,
                        (flipped_source.origin.y + flipped_source.size.height) / texture_size.height);
    
    f32 uv_scale_x = 1;
    f32 uv_scale_y = 1;
    
    if (repeat) {
        // in order to repeat a texture, we need to scale the uv's to be larger than the actual source
        if (size.width > flipped_source.size.width) {
            uv_scale_x = size.width / flipped_source.size.width;
        }
        
        if (size.height > flipped_source.size.height) {
            uv_scale_y = size.height / flipped_source.size.height;
        }
    }
    
    SHIZVector2 const uv_min_scaled = SHIZVector2Make(uv_min.x * uv_scale_x,
                                                      uv_min.y * uv_scale_y);
    SHIZVector2 const uv_max_scaled = SHIZVector2Make(uv_max.x * uv_scale_x,
                                                      uv_max.y * uv_scale_y);
    
    bool const flip_vertically = (flip & SHIZSpriteFlipModeVertical) == SHIZSpriteFlipModeVertical;
    bool const flip_horizontally = (flip & SHIZSpriteFlipModeHorizontal) == SHIZSpriteFlipModeHorizontal;
    
    SHIZVector2 const tl = SHIZVector2Make(flip_horizontally ? uv_max_scaled.x : uv_min_scaled.x,
                                           flip_vertically ? uv_min_scaled.y : uv_max_scaled.y);
    SHIZVector2 const br = SHIZVector2Make(flip_horizontally ? uv_min_scaled.x : uv_max_scaled.x,
                                           flip_vertically ? uv_max_scaled.y : uv_min_scaled.y);
    SHIZVector2 const bl = SHIZVector2Make(flip_horizontally ? uv_max_scaled.x : uv_min_scaled.x,
                                           flip_vertically ? uv_max_scaled.y : uv_min_scaled.y);
    SHIZVector2 const tr = SHIZVector2Make(flip_horizontally ? uv_min_scaled.x : uv_max_scaled.x,
                                           flip_vertically ? uv_min_scaled.y : uv_max_scaled.y);
    
    sprite->vertices[0].texture_coord = tl;
    sprite->vertices[1].texture_coord = br;
    sprite->vertices[2].texture_coord = bl;
    
    sprite->vertices[3].texture_coord = tl;
    sprite->vertices[4].texture_coord = tr;
    sprite->vertices[5].texture_coord = br;
    
    for (u8 vertex = 0; vertex < SHIZSpriteVertexCount; vertex++) {
        sprite->vertices[vertex].color = tint;
        // in order for repeated textures to work (without having to set wrapping modes,
        // and with support for sub-textures) we have to specify the space that
        // uv's are limited to (otherwise a sub-texture with a scaled uv would
        // just end up using part of another subtexture- we don't want that) so this solution
        // will simply "loop over" a scaled uv coordinate so that it is restricted
        // within the dimensions of the expected texture
        sprite->vertices[vertex].texture_coord_min = uv_min;
        sprite->vertices[vertex].texture_coord_max = uv_max;
    }
}

#ifdef SHIZ_DEBUG

u32
z_debug__get_sprite_count()
{
    return _sprite_list.total;
}

#endif
