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
#include <SHIZEN/loader.h>

#include <limits.h>

#include "debug.h"
#include "res.h"
#include "io.h"

#include "spritefont.8x8.h"

#define SHIZDebugEventMax 64

typedef struct SHIZDebugContext {
    SHIZSpriteFont font;
    SHIZDebugEvent events[SHIZDebugEventMax];
    unsigned int event_count;
    bool is_enabled;
    bool is_expanded;
    bool is_events_enabled; // used to disable event/draw call tracking while drawing debug stuff
    bool draw_shapes;
    bool draw_events;
    bool print_sprite_order;
} SHIZDebugContext;

static void _shiz_debug_update_frame_averages(void);
static bool _shiz_debug_load_font(void);

static SHIZDebugContext _context;
static SHIZDebugFrameStats _frame_stats;

static double const _frame_average_interval = 1.0; // in seconds

static double _last_frame_time = 0;
static double _last_average_time = 0;

static unsigned int _frame_samples = 0; // sample frames to calculate average
static unsigned int _frame_sample_count = 0;

static double _frame_time = 0;
static double _frame_time_avg = 0;

static double _frame_time_samples = 0; // sample frames to calculate average
static unsigned int _frame_time_sample_count = 0;

bool
shiz_debug_init()
{
    _context.is_enabled = false;
    _context.is_events_enabled = true;
    _context.draw_shapes = false;
    _context.draw_events = false;
    _context.print_sprite_order = false;
    _context.event_count = 0;

    _frame_stats.draw_count = 0;
    _frame_stats.frame_time = 0;
    _frame_stats.frame_time_avg = 0;
    _frame_stats.frames_per_second = 0;
    _frame_stats.frames_per_second_min = UINT_MAX;
    _frame_stats.frames_per_second_max = 0;
    _frame_stats.frames_per_second_avg = 0;

    if (!_shiz_debug_load_font()) {
        return false;
    }

    return true;
}

bool
shiz_debug_kill()
{
    _context.is_enabled = false;

    if (!shiz_res_debug_unload_font()) {
        return false;
    }
    
    return true;
}

void
shiz_debug_process_errors()
{
    GLenum error;
    
    while ((error = glGetError()) != GL_NO_ERROR) {
        z_io__error_context("OPENGL", "%d", error);
    }
}

void
shiz_debug_toggle_enabled()
{
    _context.is_enabled = !_context.is_enabled;
}

void shiz_debug_toggle_expanded(bool const expanded)
{
    _context.is_expanded = expanded;
}

void
shiz_debug_toggle_events_enabled()
{
    shiz_debug_set_events_enabled(!_context.is_events_enabled);
}

void
shiz_debug_toggle_draw_events()
{
    _context.draw_events = !_context.draw_events;
}

void
shiz_debug_toggle_draw_shapes()
{
    _context.draw_shapes = !_context.draw_shapes;
}

bool
shiz_debug_is_enabled()
{
    return _context.is_enabled;
}

bool
shiz_debug_is_expanded()
{
    return _context.is_expanded;
}

bool
shiz_debug_is_events_enabled()
{
    return _context.is_events_enabled;
}

bool
shiz_debug_is_drawing_events()
{
    return _context.draw_events;
}

bool
shiz_debug_is_drawing_shapes()
{
    return _context.draw_shapes;
}

bool
shiz_debug_is_printing_sprite_order()
{
    return _context.print_sprite_order;
}

void
shiz_debug_set_drawing_shapes(bool const enabled)
{
    _context.draw_shapes = enabled;
}

void
shiz_debug_set_events_enabled(bool const enabled)
{
    _context.is_events_enabled = enabled;
}

void
shiz_debug_set_is_printing_sprite_order(bool const enabled)
{
    _context.print_sprite_order = enabled;
}

SHIZSpriteFont
shiz_debug_get_font()
{
    return _context.font;
}

unsigned int
shiz_debug_get_event_count()
{
    return _context.event_count;
}

void
shiz_debug_reset_events()
{
    _context.event_count = 0;
}

void
shiz_debug_add_event(SHIZDebugEvent const event)
{
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
shiz_debug_add_event_resource(const char * const filename, SHIZVector3 const origin)
{
    if (filename) {
        SHIZDebugEvent event;

        event.name = filename;
        event.origin = origin;
        event.lane = SHIZDebugEventLaneResources;

        shiz_debug_add_event(event);
    }
}

void
shiz_debug_add_event_draw(const char * const cause, SHIZVector3 const origin)
{
    if (cause) {
        SHIZDebugEvent event;

        event.name = cause;
        event.origin = origin;
        event.lane = SHIZDebugEventLaneDraws;

        shiz_debug_add_event(event);
    }
}

SHIZDebugEvent
shiz_debug_get_event(unsigned int const index)
{
    return _context.events[index];
}

void
shiz_debug_reset_draw_count()
{
    _frame_stats.draw_count = 0;
}

void
shiz_debug_increment_draw_count(unsigned int amount)
{
    if (shiz_debug_is_events_enabled()) {
        _frame_stats.draw_count += amount;
    }
}

void
shiz_debug_update_frame_stats()
{
    double const time = glfwGetTime();
    double const time_since_last_frame = time - _last_frame_time;

    _frame_time = time_since_last_frame;
    _last_frame_time = time;

    _frame_stats.frames_per_second = (unsigned int)(1.0 / _frame_time);

    if (_frame_stats.frames_per_second < _frame_stats.frames_per_second_min) {
        _frame_stats.frames_per_second_min = _frame_stats.frames_per_second;
    }

    if (_frame_stats.frames_per_second > _frame_stats.frames_per_second_max) {
        _frame_stats.frames_per_second_max = _frame_stats.frames_per_second;
    }

    _frame_samples += _frame_stats.frames_per_second;
    _frame_sample_count++;

    _frame_time_samples += _frame_time;
    _frame_time_sample_count++;

    double const time_since_last_average = time - _last_average_time;

    if (time_since_last_average >= _frame_average_interval) {
        _last_average_time = time;

        _shiz_debug_update_frame_averages();
    }

    _frame_stats.frame_time = _frame_time * 1000;
    _frame_stats.frame_time_avg = _frame_time_avg * 1000;
}

SHIZDebugFrameStats
shiz_debug_get_frame_stats()
{
    return _frame_stats;
}

static
void
_shiz_debug_update_frame_averages()
{
    _frame_stats.frames_per_second_avg = _frame_samples / _frame_sample_count;
    _frame_sample_count = 0;
    _frame_samples = 0;

    _frame_time_avg = _frame_time_samples / _frame_time_sample_count;
    _frame_time_sample_count = 0;
    _frame_time_samples = 0;

    // reset min/max to show rolling stats rather than historically accurate stats (its more interesting
    // knowing min/max for the current scene/context than knowing the 9999+ max fps during the first blank frame)
    _frame_stats.frames_per_second_min = UINT_MAX;
    _frame_stats.frames_per_second_max = 0;
}

static
bool
_shiz_debug_load_font()
{
    if (!shiz_res_debug_load_font(IBM8x8, IBM8x8Size)) {
        return false;
    }

    SHIZSprite const sprite = z_load_sprite_from(shiz_res_debug_get_font());

    if (sprite.resource_id != SHIZResourceInvalid) {
        SHIZSpriteFont const spritefont = z_load_spritefont_from(sprite, SHIZSizeMake(IBM8x8TileSize, IBM8x8TileSize));

        _context.font = spritefont;
        _context.font.table.codepage = IBM8x8Codepage;
    }

    return true;
}
#endif
