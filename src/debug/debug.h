#pragma once

#ifdef SHIZ_DEBUG

#include "../internal.h"

#include "sprite_debug.h"
#include "draw_debug.h"

#include <stdint.h> // uint8_t, uint32_t

bool z_debug__init(void);
bool z_debug__kill(void);

void z_debug__toggle_enabled(void);
void z_debug__toggle_expanded(bool expanded);
void z_debug__toggle_draw_shapes(void);
void z_debug__toggle_draw_axes(void);

bool z_debug__is_enabled(void);
bool z_debug__is_expanded(void);
bool z_debug__is_drawing_axes(void);
bool z_debug__is_drawing_shapes(void);
bool z_debug__is_printing_sprite_order(void);

void z_debug__set_drawing_shapes(bool enabled);
void z_debug__set_is_printing_sprite_order(bool enabled);

bool z_debug__load_font(uint8_t const * buffer, uint32_t length);
bool z_debug__unload_font(void);

SHIZSpriteFont z_debug__get_font(void);
uint8_t z_debug__get_font_resource(void);

#endif
