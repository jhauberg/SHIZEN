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

#include "gfx.h"
#include "res.h"

#define SHIZSpriteInternalVertexCount 6

typedef struct SHIZSpriteInternal {
    unsigned long key; // packed SHIZSpriteInternalKey
    unsigned int order; // literal call order; used as a last resort to ensure stable sorting
    SHIZVertexPositionColorTexture vertices[SHIZSpriteInternalVertexCount];
    SHIZVector3 origin;
    float angle;
} SHIZSpriteInternal;

typedef struct SHIZSpriteInternalKey {
    bool is_transparent: 1; // the least significant bit
    unsigned short texture_id: 7;
    SHIZLayer layer; // 24 bits
} SHIZSpriteInternalKey; // total 32 bits (unsigned long)

static int _shiz_sprite_compare(const void * a, const void * b);
static void _shiz_sprite_sort(void);

static void _shiz_sprite_set_position(SHIZSpriteInternal * sprite_internal, SHIZSize const size, SHIZVector2 const anchor);
static void _shiz_sprite_set_uv(SHIZSpriteInternal * sprite_internal, SHIZSize const size, SHIZSize const texture_size, SHIZRect const source, SHIZColor const tint, bool const repeat);

static unsigned int _sprites_count = 0;
static unsigned int _sprites_count_total = 0;

static SHIZSpriteInternal _sprites[SHIZSpriteInternalMax];

SHIZSize const
shiz_sprite_draw(SHIZSprite const sprite,
                 SHIZVector2 const origin,
                 SHIZSize const size,
                 SHIZVector2 const anchor,
                 float const angle,
                 SHIZColor const tint,
                 bool const repeat,
                 bool const opaque,
                 SHIZLayer const layer)
{
    SHIZResourceImage const image = shiz_res_get_image(sprite.resource_id);

    if (sprite.resource_id == SHIZResourceInvalid ||
        sprite.resource_id != image.resource_id ||
        (sprite.source.size.width <= 0 ||
         sprite.source.size.height <= 0)) {
        return SHIZSizeZero;
    }

    float const z = _shiz_layer_get_z(layer);
    
    unsigned long sort_key = 0;
    
    SHIZSpriteInternalKey * sprite_key = (SHIZSpriteInternalKey *)&sort_key;
    
    sprite_key->layer = layer;
    sprite_key->texture_id = image.texture_id;
    sprite_key->is_transparent = !opaque;
    
    SHIZSpriteInternal * sprite_internal = &_sprites[_sprites_count];
    
    sprite_internal->key = sort_key;
    sprite_internal->angle = angle;
    sprite_internal->origin = SHIZVector3Make(origin.x, origin.y, z);
    sprite_internal->order = _sprites_count_total;

    SHIZSize const texture_size = SHIZSizeMake(image.width, image.height);
    SHIZSize const sprite_size = (size.width == SHIZSpriteSizeIntrinsic.width &&
                                  size.height == SHIZSpriteSizeIntrinsic.height) ?
                                    sprite.source.size : size;
    
    // set vertex positions appropriately for the given anchor (note that vertices are not transformed until flushed)
    _shiz_sprite_set_position(sprite_internal, sprite_size, anchor);
    // set texture coordinates appropriately, taking repeating/tiling into account
    _shiz_sprite_set_uv(sprite_internal, sprite_size, texture_size, sprite.source, tint, repeat);

    // count for current batch
    _sprites_count += 1;
    // count for total sprites during a frame; i.e. the accumulation of all flushed sprites
    _sprites_count_total += 1;

    if (_sprites_count >= SHIZSpriteInternalMax) {
        shiz_sprite_flush();
    }

    return sprite_size;
}

SHIZRect const
shiz_sprite_get_anchored_rect(SHIZSize const size, SHIZVector2 const anchor)
{
    float const hw = size.width / 2;
    float const hh = size.height / 2;

    float const dx = hw * -anchor.x;
    float const dy = hh * -anchor.y;

    float const l = dx - hw;
    float const b = dy - hh;

    return SHIZRectMake(SHIZVector2Make(l, b), size);
}

void
shiz_sprite_reset()
{
    _sprites_count = 0;
    _sprites_count_total = 0;
}

void
shiz_sprite_flush()
{
    if (_sprites_count == 0) {
        return;
    }

    _shiz_sprite_sort();

    for (unsigned int sprite_index = 0; sprite_index < _sprites_count; sprite_index++) {
        SHIZSpriteInternal const sprite = _sprites[sprite_index];
        SHIZSpriteInternalKey * const sprite_key = (SHIZSpriteInternalKey *)&sprite.key;

        // finally push vertex data to the renderer
        shiz_gfx_render_quad(sprite.vertices,
                             sprite.origin,
                             sprite.angle,
                             sprite_key->texture_id);
    }

    _sprites_count = 0;
}

static int
_shiz_sprite_compare(const void * a, const void * b)
{
    SHIZSpriteInternal const * lhs = (SHIZSpriteInternal *)a;
    SHIZSpriteInternal const * rhs = (SHIZSpriteInternal *)b;
    
    if (lhs->key < rhs->key) {
        return -1;
    } else if (lhs->key > rhs->key) {
        return 1;
    } else if (lhs->order < rhs->order) {
        return -1;
    } else if (lhs->order > rhs->order) {
        return 1;
    }
    
    return 0;
}

static void
_shiz_sprite_sort(void)
{
    // sort sprites based on their layer parameters, but also optimized for reduced state switching
    qsort(_sprites,
          _sprites_count,
          sizeof(SHIZSpriteInternal),
          _shiz_sprite_compare);
    
    // note that sorting can not guarantee correct order in cases where flushing is required due to
    // reaching sprite capacity (though the z-buffer should help)
}

static void
_shiz_sprite_set_position(SHIZSpriteInternal * sprite_internal,
                          SHIZSize const size,
                          SHIZVector2 const anchor)
{
    SHIZRect const anchored_rect = shiz_sprite_get_anchored_rect(size, anchor);
    
    float const l = anchored_rect.origin.x;
    float const r = anchored_rect.origin.x + anchored_rect.size.width;
    float const b = anchored_rect.origin.y;
    float const t = anchored_rect.origin.y + anchored_rect.size.height;
    
    SHIZVector2 const bl = SHIZVector2Make(l, b);
    SHIZVector2 const tl = SHIZVector2Make(l, t);
    SHIZVector2 const tr = SHIZVector2Make(r, t);
    SHIZVector2 const br = SHIZVector2Make(r, b);
    
    sprite_internal->vertices[0].position = SHIZVector3Make(tl.x, tl.y, 0);
    sprite_internal->vertices[1].position = SHIZVector3Make(br.x, br.y, 0);
    sprite_internal->vertices[2].position = SHIZVector3Make(bl.x, bl.y, 0);
    
    sprite_internal->vertices[3].position = SHIZVector3Make(tl.x, tl.y, 0);
    sprite_internal->vertices[4].position = SHIZVector3Make(tr.x, tr.y, 0);
    sprite_internal->vertices[5].position = SHIZVector3Make(br.x, br.y, 0);
}

static void
_shiz_sprite_set_uv(SHIZSpriteInternal * sprite_internal,
                    SHIZSize const size,
                    SHIZSize const texture_size,
                    SHIZRect const source,
                    SHIZColor const tint,
                    bool const repeat)
{
    bool const flip_vertically = true;
    
    SHIZRect flipped_source = source;
    
    if (flip_vertically) {
        // opengl assumes that the origin of textures is at the bottom-left of the image,
        // however, it is common to specify top-left as origin when using e.g. sprite sheets (and we want that)
        // so, assuming that the provided source frame expects the top-left to be the origin,
        // we have to flip the specified coordinate so that the origin becomes bottom-left
        flipped_source.origin.y = (texture_size.height - source.size.height) - source.origin.y;
    }
    
    SHIZVector2 const uv_min = SHIZVector2Make((flipped_source.origin.x / texture_size.width),
                                               (flipped_source.origin.y / texture_size.height));
    SHIZVector2 const uv_max = SHIZVector2Make(((flipped_source.origin.x + flipped_source.size.width) / texture_size.width),
                                               ((flipped_source.origin.y + flipped_source.size.height) / texture_size.height));
    
    float uv_scale_x = 1;
    float uv_scale_y = 1;
    
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
    
    SHIZVector2 const tl = SHIZVector2Make(uv_min_scaled.x, uv_max_scaled.y);
    SHIZVector2 const br = SHIZVector2Make(uv_max_scaled.x, uv_min_scaled.y);
    SHIZVector2 const bl = SHIZVector2Make(uv_min_scaled.x, uv_min_scaled.y);
    SHIZVector2 const tr = SHIZVector2Make(uv_max_scaled.x, uv_max_scaled.y);
    
    sprite_internal->vertices[0].texture_coord = tl;
    sprite_internal->vertices[1].texture_coord = br;
    sprite_internal->vertices[2].texture_coord = bl;
    
    sprite_internal->vertices[3].texture_coord = tl;
    sprite_internal->vertices[4].texture_coord = tr;
    sprite_internal->vertices[5].texture_coord = br;
    
    for (unsigned int i = 0; i < SHIZSpriteInternalVertexCount; i++) {
        sprite_internal->vertices[i].color = tint;
        // in order for repeated textures to work (without having to set wrapping modes,
        // and with support for sub-textures) we have to specify the space that
        // uv's are limited to (otherwise a sub-texture with a scaled uv would
        // just end up using part of another subtexture- we don't want that) so this solution
        // will simply "loop over" a scaled uv coordinate so that it is restricted
        // within the dimensions of the expected texture
        sprite_internal->vertices[i].texture_coord_min = uv_min;
        sprite_internal->vertices[i].texture_coord_max = uv_max;
    }
}

#ifdef SHIZ_DEBUG
unsigned int
shiz_debug_get_sprite_count()
{
    return _sprites_count_total;
}
#endif
