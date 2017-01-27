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

static int _shiz_compare_sprites(const void * a, const void * b);

static unsigned int _sprites_count = 0;

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
                 SHIZLayer const layer) {
    SHIZResourceImage const image = shiz_res_get_image(sprite.resource_id);

    if (sprite.resource_id == SHIZResourceInvalid ||
        sprite.resource_id != image.resource_id ||
        (sprite.source.size.width <= 0 ||
         sprite.source.size.height <= 0)) {
        return SHIZSizeEmpty;
    }

    unsigned int const vertex_count = 6;

    SHIZVertexPositionColorTexture vertices[vertex_count];

    for (unsigned int i = 0; i < vertex_count; i++) {
        vertices[i].color = tint;
    }

    SHIZSize const working_size = (size.width == SHIZSpriteSizeIntrinsic.width &&
                                   size.height == SHIZSpriteSizeIntrinsic.height) ? sprite.source.size : size;

    SHIZRect const anchored_rect = shiz_sprite_get_anchored_rect(working_size, anchor);

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

    for (unsigned int i = 0; i < vertex_count; i++) {
        // in order for repeated textures to work (without having to set wrapping modes,
        // and with support for sub-textures) we have to specify the space that
        // uv's are limited to (otherwise a sub-texture with a scaled uv would
        // just end up using part of another subtexture- we don't want that) so this solution
        // will simply "loop over" a scaled uv coordinate so that it is restricted
        // within the dimensions of the expected texture
        vertices[i].texture_coord_min = uv_min;
        vertices[i].texture_coord_max = uv_max;
    }

    float const z = _shiz_layer_get_z(layer);

    unsigned long key = 0;

    SHIZSpriteInternalKey * sprite_key = (SHIZSpriteInternalKey *)&key;
    
    sprite_key->layer = layer;
    sprite_key->texture_id = image.texture_id;
    sprite_key->is_transparent = !opaque;

    SHIZSpriteInternal * sprite_internal = &_sprites[_sprites_count];

    sprite_internal->key = key;
    sprite_internal->angle = angle;
    sprite_internal->origin = SHIZVector3Make(origin.x, origin.y, z);
    sprite_internal->order = _sprites_count;

    for (unsigned int i = 0; i < vertex_count; i++) {
        sprite_internal->vertices[i].position = vertices[i].position;
        sprite_internal->vertices[i].texture_coord = vertices[i].texture_coord;
        sprite_internal->vertices[i].texture_coord_max = vertices[i].texture_coord_max;
        sprite_internal->vertices[i].texture_coord_min = vertices[i].texture_coord_min;
        sprite_internal->vertices[i].color = vertices[i].color;
    }

    _sprites_count += 1;

#ifdef SHIZ_DEBUG
    shiz_debug_context.sprite_count += 1;
#endif

    if (_sprites_count >= SHIZSpriteInternalMax) {
        shiz_sprite_flush();
    }

    return anchored_rect.size;
}

SHIZRect const
shiz_sprite_get_anchored_rect(SHIZSize const size, SHIZVector2 const anchor) {
    float const hw = size.width / 2;
    float const hh = size.height / 2;

    float const dx = hw * -anchor.x;
    float const dy = hh * -anchor.y;

    float const l = dx - hw;
    float const b = dy - hh;

    return SHIZRectMake(SHIZVector2Make(l, b), size);
}

void
shiz_sprite_flush() {
    if (_sprites_count == 0) {
        return;
    }

    // sort sprites based on their layer parameters, but also optimized for reduced state switching
    qsort(_sprites, _sprites_count, sizeof(SHIZSpriteInternal),
          _shiz_compare_sprites);
    // note that sorting can not guarantee correct order in cases where flushing is required due to
    // reaching sprite capacity (though the z-buffer should help)

    for (unsigned int sprite_index = 0; sprite_index < _sprites_count; sprite_index++) {
        SHIZSpriteInternal const sprite = _sprites[sprite_index];
        SHIZSpriteInternalKey * const sprite_key = (SHIZSpriteInternalKey *)&sprite.key;

        shiz_gfx_render_quad(sprite.vertices, sprite.origin, sprite.angle, sprite_key->texture_id);
    }

    _sprites_count = 0;
}

static int
_shiz_compare_sprites(const void * a, const void * b) {
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
