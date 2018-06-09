#pragma once

#include <stdint.h> // uint32_t

#include "../internal.h" // SHIZVertexPositionColor, SHIZVector3, GLenum

bool z_gfx__init_immediate(void);
bool z_gfx__kill_immediate(void);

void z_gfx__render_immediate(GLenum mode, SHIZVertexPositionColor const * restrict vertices, uint32_t count, SHIZVector3 origin, float angle);
