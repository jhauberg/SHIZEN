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

#ifdef SHIZ_DEBUG

#include <SHIZEN/zloader.h>

#include "debug.h"

#include "../res.h"
#include "../io.h"

#include "../spritefont.8x8.h"

#define SHIZDebugEventMax 64

char const * const SHIZDebugEventNamePrimitive = "◘";
char const * const SHIZDebugEventNameFlush = "fls";
char const * const SHIZDebugEventNameFlushByCapacity = "fls|cap";
char const * const SHIZDebugEventNameFlushByTextureSwitch = "fls|tex";

typedef struct SHIZDebugContext {
    SHIZSpriteFont font;
    SHIZDebugEvent events[SHIZDebugEventMax];
    uint16_t event_count;
    bool is_enabled;
    bool is_expanded;
    bool is_events_enabled;
    bool draw_shapes;
    bool draw_events;
    bool draw_axes;
    bool print_sprite_order;
    uint8_t pad[7];
} SHIZDebugContext;

static
bool
z_debug__prepare_font(void);

static SHIZDebugContext _context;

bool
z_debug__init()
{
    _context.is_enabled = false;
    _context.is_events_enabled = true;
    _context.draw_shapes = false;
    _context.draw_events = false;
    _context.draw_axes = false;
    _context.print_sprite_order = false;
    _context.event_count = 0;

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
z_debug__toggle_events_enabled()
{
    z_debug__set_events_enabled(!_context.is_events_enabled);
}

void
z_debug__toggle_draw_events()
{
    _context.draw_events = !_context.draw_events;
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
z_debug__is_events_enabled()
{
    return _context.is_events_enabled;
}

bool
z_debug__is_drawing_events()
{
    return _context.draw_events;
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
z_debug__set_events_enabled(bool const enabled)
{
    _context.is_events_enabled = enabled;
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

uint16_t
z_debug__get_event_count()
{
    return _context.event_count;
}

void
z_debug__reset_events()
{
    _context.event_count = 0;
}

void
z_debug__add_event(SHIZDebugEvent const event)
{
    if (z_debug__is_events_enabled()) {
        if (z_debug__get_event_count() < SHIZDebugEventMax) {
            _context.events[_context.event_count].name = event.name;
            _context.events[_context.event_count].lane = event.lane;
            _context.events[_context.event_count].origin = event.origin;
            
            _context.event_count += 1;
        } else {
            //printf("event not shown: %s\n", event.name);
        }
    }
}

void
z_debug__add_event_resource(char const * const filename,
                            SHIZVector3 const origin)
{
    if (filename) {
        SHIZDebugEvent event;

        event.name = filename;
        event.origin = origin;
        event.lane = SHIZDebugEventLaneResources;

        z_debug__add_event(event);
    }
}

void
z_debug__add_event_draw(char const * const cause,
                        SHIZVector3 const origin)
{
    if (cause) {
        SHIZDebugEvent event;

        event.name = cause;
        event.origin = origin;
        event.lane = SHIZDebugEventLaneDraws;

        z_debug__add_event(event);
    }
}

SHIZDebugEvent
z_debug__get_event(uint16_t const index)
{
    return _context.events[index];
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
