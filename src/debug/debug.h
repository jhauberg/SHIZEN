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

#pragma once

#ifdef SHIZ_DEBUG

#include "../internal.h"

#include "sprite_debug.h"
#include "spritebatch_debug.h"
#include "draw_debug.h"

#define SHIZDebugEventLaneDraws 0
#define SHIZDebugEventLaneResources 1

typedef struct SHIZDebugEvent {
    char const * name;
    SHIZVector3 origin;
    uint8_t lane;
} SHIZDebugEvent;

extern char const * const SHIZDebugEventNamePrimitive;
extern char const * const SHIZDebugEventNameFlush;
extern char const * const SHIZDebugEventNameFlushByCapacity;
extern char const * const SHIZDebugEventNameFlushByTextureSwitch;

bool z_debug__init(void);
bool z_debug__kill(void);

void z_debug__toggle_enabled(void);
void z_debug__toggle_expanded(bool expanded);
void z_debug__toggle_events_enabled(void);
void z_debug__toggle_draw_events(void);
void z_debug__toggle_draw_shapes(void);
void z_debug__toggle_draw_axes(void);

bool z_debug__is_enabled(void);
bool z_debug__is_expanded(void);
bool z_debug__is_events_enabled(void);
bool z_debug__is_drawing_events(void);
bool z_debug__is_drawing_axes(void);
bool z_debug__is_drawing_shapes(void);
bool z_debug__is_printing_sprite_order(void);

void z_debug__set_drawing_shapes(bool enabled);
void z_debug__set_events_enabled(bool enabled);
void z_debug__set_is_printing_sprite_order(bool enabled);

SHIZDebugEvent z_debug__get_event(uint16_t index);

uint16_t z_debug__get_event_count(void);

void z_debug__reset_events(void);

void z_debug__add_event(SHIZDebugEvent event);
void z_debug__add_event_resource(char const * filename, SHIZVector3 origin);
void z_debug__add_event_draw(char const * cause, SHIZVector3 origin);

void z_debug__print_resources(void);

bool z_debug__load_font(uint8_t const * buffer, uint32_t length);
bool z_debug__unload_font(void);

SHIZSpriteFont z_debug__get_font(void);
uint8_t z_debug__get_font_resource(void);

#endif
