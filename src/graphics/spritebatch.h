#pragma once

#include <stdbool.h> // bool

#include "../internal.h" // SHIZVertexPositionColorText, SHIZVector3, GLuint

bool z_gfx__init_spritebatch(void);
bool z_gfx__kill_spritebatch(void);

void z_gfx__add_sprite(SHIZVertexPositionColorTexture const * restrict vertices, SHIZVector3 origin, float angle, GLuint texture_id);

bool z_gfx__spritebatch_flush(void);
void z_gfx__spritebatch_reset(void);
