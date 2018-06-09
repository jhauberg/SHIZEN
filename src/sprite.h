#pragma once

#include <SHIZEN/ztype.h> // SHIZRect, SHIZSize, SHIZVector2, SHIZSprite

/**
 * The amount of sprites that can be sorted per frame before a batch is issued.
 */
#define SHIZSpriteMax 2048

SHIZRect const z_sprite__anchor_rect(SHIZSize size, SHIZVector2 anchor);

void z_sprite__reset(void);
void z_sprite__flush(void);

SHIZSize const z_sprite__draw(SHIZSprite sprite,
                              SHIZVector2 origin,
                              SHIZSpriteSize size,
                              bool repeat,
                              SHIZVector2 anchor,
                              SHIZSpriteFlipMode flip,
                              float angle,
                              SHIZColor tint,
                              bool opaque,
                              SHIZLayer layer);
