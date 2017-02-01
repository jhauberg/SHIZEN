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

#include <SHIZEN/engine.h>

#include "res.h"
#include "io.h"

#include "spritefont.8x8.h"

#define SHIZDebugEventMax 64

typedef struct SHIZDebugContext {
    bool is_enabled;
    bool is_events_enabled; // used to disable event/draw call tracking while drawing debug stuff
    bool draw_shapes;
    bool draw_events;
    unsigned int event_count;
    SHIZSpriteFont font;
    SHIZDebugEvent events[SHIZDebugEventMax];
} SHIZDebugContext;

static SHIZDebugContext _context;

bool
shiz_debug_init() {
    _context.is_enabled = true;
    _context.is_events_enabled = true;
    _context.draw_shapes = false;
    _context.draw_events = false;
    _context.event_count = 0;
    
    if (!shiz_res_debug_load_font(IBMCGA8x8, IBMCGA8x8Size)) {
        return false;
    }
    
    SHIZSprite const sprite = shiz_get_sprite(shiz_res_debug_get_font());
    
    if (sprite.resource_id != SHIZResourceInvalid) {
        SHIZSpriteFont const spritefont = shiz_get_sprite_font(sprite, SHIZSizeMake(8, 8));
        
        _context.font = spritefont;
        _context.font.table.offset = IBMCGA8x8Offset;
    }
    
    return true;
}

bool
shiz_debug_kill() {
    _context.is_enabled = false;
    
    if (!shiz_res_debug_unload_font()) {
        return false;
    }
    
    return true;
}

void
shiz_debug_process_errors() {
    GLenum error;
    
    while ((error = glGetError()) != GL_NO_ERROR) {
        shiz_io_error_context("OPENGL", "%d", error);
    }
}

void
shiz_debug_toggle_enabled() {
    _context.is_enabled = !_context.is_enabled;
}

void
shiz_debug_toggle_events_enabled() {
    shiz_debug_set_events_enabled(!_context.is_events_enabled);
}

void
shiz_debug_toggle_draw_events() {
    _context.draw_events = !_context.draw_events;
}

void
shiz_debug_toggle_draw_shapes() {
    _context.draw_shapes = !_context.draw_shapes;
}

bool
shiz_debug_is_enabled() {
    return _context.is_enabled;
}

bool
shiz_debug_is_events_enabled() {
    return _context.is_events_enabled;
}

bool
shiz_debug_is_drawing_events() {
    return _context.draw_events;
}

bool
shiz_debug_is_drawing_shapes() {
    return _context.draw_shapes;
}

void
shiz_debug_set_drawing_shapes(bool const enabled) {
    _context.draw_shapes = enabled;
}

void
shiz_debug_set_events_enabled(bool const enabled) {
    _context.is_events_enabled = enabled;
}

SHIZSpriteFont
shiz_debug_get_font() {
    return _context.font;
}

unsigned int
shiz_debug_get_event_count() {
    return _context.event_count;
}

void
shiz_debug_reset_events() {
    _context.event_count = 0;
}

void
shiz_debug_add_event(SHIZDebugEvent const event) {
    if (shiz_debug_is_events_enabled()) {
        if (shiz_debug_get_event_count() < SHIZDebugEventMax) {
            _context.events[_context.event_count].name = event.name;
            _context.events[_context.event_count].lane = event.lane;
            _context.events[_context.event_count].origin = event.origin;
            
            _context.event_count += 1;
        } else {
            printf("event not shown: %s", event.name);
        }
    }
}

void
shiz_debug_add_event_resource(const char * const filename, SHIZVector3 const origin) {
    if (filename) {
        SHIZDebugEvent event;

        event.name = filename;
        event.origin = origin;
        event.lane = SHIZDebugEventLaneResources;

        shiz_debug_add_event(event);
    }
}

void
shiz_debug_add_event_draw(const char * const cause, SHIZVector3 const origin) {
    if (cause) {
        SHIZDebugEvent event;

        event.name = cause;
        event.origin = origin;
        event.lane = SHIZDebugEventLaneDraws;

        shiz_debug_add_event(event);
    }
}

SHIZDebugEvent
shiz_debug_get_event(unsigned int const index) {
    return _context.events[index];
}
