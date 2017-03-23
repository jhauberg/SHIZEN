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

#include <SHIZEN/type.h>

#define SHIZDebugEventLaneDraws 0
#define SHIZDebugEventLaneResources 1

typedef struct SHIZDebugEvent {
    SHIZVector3 origin;
    const char * name;
    unsigned int lane;
} SHIZDebugEvent;

typedef struct SHIZDebugFrameStats {
    unsigned int draw_count;
    unsigned int frames_per_second;
    unsigned int frames_per_second_min;
    unsigned int frames_per_second_max;
    unsigned int frames_per_second_avg;
    double frame_time;
    double frame_time_avg;
} SHIZDebugFrameStats;

bool shiz_debug_init(void);
bool shiz_debug_kill(void);

void shiz_debug_process_errors(void);

void shiz_debug_toggle_enabled(void);
void shiz_debug_toggle_events_enabled(void);
void shiz_debug_toggle_draw_events(void);
void shiz_debug_toggle_draw_shapes(void);

bool shiz_debug_is_enabled(void);
bool shiz_debug_is_events_enabled(void);
bool shiz_debug_is_drawing_events(void);
bool shiz_debug_is_drawing_shapes(void);

void shiz_debug_set_drawing_shapes(bool const enabled);
void shiz_debug_set_events_enabled(bool const enabled);

SHIZSpriteFont shiz_debug_get_font(void);
SHIZDebugEvent shiz_debug_get_event(unsigned int const index);

unsigned int shiz_debug_get_event_count(void);

void shiz_debug_reset_events(void);
void shiz_debug_add_event(SHIZDebugEvent const event);
void shiz_debug_add_event_resource(const char * const filename, SHIZVector3 const origin);
void shiz_debug_add_event_draw(const char * const cause, SHIZVector3 const origin);

void shiz_debug_reset_draw_count(void);
void shiz_debug_increment_draw_count(unsigned int amount);
void shiz_debug_update_frame_stats(void);

SHIZDebugFrameStats shiz_debug_get_frame_stats(void);

#endif // debug_h
