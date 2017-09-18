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

#ifndef debug_h
#define debug_h

#ifdef SHIZ_DEBUG

#include "../internal_type.h"

#include "sprite_debug.h"
#include "spritebatch_debug.h"
#include "draw_debug.h"

#define SHIZDebugEventLaneDraws 0
#define SHIZDebugEventLaneResources 1

typedef struct SHIZDebugEvent {
    SHIZVector3 origin;
    char const * name;
    unsigned int lane;
} SHIZDebugEvent;

extern char const * const SHIZDebugEventNamePrimitive;
extern char const * const SHIZDebugEventNameFlush;
extern char const * const SHIZDebugEventNameFlushByCapacity;
extern char const * const SHIZDebugEventNameFlushByTextureSwitch;

typedef struct SHIZDebugFrameStats {
    unsigned int draw_count;
    unsigned int frames_per_second;
    unsigned int frames_per_second_min;
    unsigned int frames_per_second_max;
    unsigned int frames_per_second_avg;
    double frame_time;
    double frame_time_avg;
} SHIZDebugFrameStats;

bool z_debug__init(void);
bool z_debug__kill(void);

void z_debug__process_errors(void);

void z_debug__toggle_enabled(void);
void z_debug__toggle_expanded(bool expanded);
void z_debug__toggle_events_enabled(void);
void z_debug__toggle_draw_events(void);
void z_debug__toggle_draw_shapes(void);

bool z_debug__is_enabled(void);
bool z_debug__is_expanded(void);
bool z_debug__is_events_enabled(void);
bool z_debug__is_drawing_events(void);
bool z_debug__is_drawing_shapes(void);
bool z_debug__is_printing_sprite_order(void);

void z_debug__set_drawing_shapes(bool enabled);
void z_debug__set_events_enabled(bool enabled);
void z_debug__set_is_printing_sprite_order(bool enabled);

SHIZSpriteFont z_debug__get_font(void);
SHIZDebugEvent z_debug__get_event(unsigned int index);

unsigned int z_debug__get_event_count(void);

void z_debug__reset_events(void);
void z_debug__add_event(SHIZDebugEvent event);
void z_debug__add_event_resource(char const * filename, SHIZVector3 origin);
void z_debug__add_event_draw(char const * cause, SHIZVector3 origin);

void z_debug__reset_draw_count(void);
void z_debug__increment_draw_count(unsigned int amount);
void z_debug__update_frame_stats(void);

SHIZDebugFrameStats z_debug__get_frame_stats(void);

void z_debug__print_resources(void);

bool z_debug__load_font(unsigned char const * buffer, unsigned int length);
bool z_debug__unload_font(void);

unsigned int z_debug__get_font_resource(void);

#endif

#endif // debug_h
