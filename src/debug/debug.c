#ifdef SHIZ_DEBUG

#include <SHIZEN/zloader.h>

#include "debug.h"

#include "../res.h"

#include "../spritefont.8x8.h"

typedef struct SHIZDebugContext {
    SHIZSpriteFont font;
    bool is_enabled;
    bool is_expanded;
    bool draw_shapes;
    bool draw_axes;
    bool print_sprite_order;
} SHIZDebugContext;

static
bool
z_debug__prepare_font(void);

static SHIZDebugContext _context;

bool
z_debug__init()
{
    _context.is_enabled = false;
    _context.draw_shapes = false;
    _context.draw_axes = false;
    _context.print_sprite_order = false;

    if (!z_debug__prepare_font()) {
        return false;
    }

    return true;
}

bool
z_debug__kill()
{
    _context.is_enabled = false;

    if (!z_debug__unload_font()) {
        return false;
    }
    
    return true;
}

void
z_debug__toggle_enabled()
{
    _context.is_enabled = !_context.is_enabled;
}

void
z_debug__toggle_expanded(bool const expanded)
{
    _context.is_expanded = expanded;
}

void
z_debug__toggle_draw_shapes()
{
    _context.draw_shapes = !_context.draw_shapes;
}

void
z_debug__toggle_draw_axes()
{
    _context.draw_axes = !_context.draw_axes;
}

bool
z_debug__is_enabled()
{
    return _context.is_enabled;
}

bool
z_debug__is_expanded()
{
    return _context.is_expanded;
}

bool
z_debug__is_drawing_shapes()
{
    return _context.draw_shapes;
}

bool
z_debug__is_drawing_axes()
{
    return _context.draw_axes;
}

bool
z_debug__is_printing_sprite_order()
{
    return _context.print_sprite_order;
}

void
z_debug__set_drawing_shapes(bool const enabled)
{
    _context.draw_shapes = enabled;
}

void
z_debug__set_is_printing_sprite_order(bool const enabled)
{
    _context.print_sprite_order = enabled;
}

SHIZSpriteFont
z_debug__get_font()
{
    return _context.font;
}

static
bool
z_debug__prepare_font()
{
    if (!z_debug__load_font(IBM8x8, IBM8x8Size)) {
        return false;
    }

    SHIZSprite const sprite = z_load_sprite_from(z_debug__get_font_resource());

    if (sprite.resource_id != SHIZResourceInvalid) {
        SHIZSize const character_size = SHIZSizeMake(IBM8x8TileSize,
                                                     IBM8x8TileSize);
        
        SHIZSpriteFont const spritefont = z_load_spritefont_from(sprite,
                                                                 character_size);

        _context.font = spritefont;
        _context.font.table.codepage = IBM8x8Codepage;
    }

    return true;
}

#endif
