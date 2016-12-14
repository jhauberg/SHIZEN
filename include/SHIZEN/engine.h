////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2016 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#ifndef engine_h
#define engine_h

#define SHIZEN_VERSION_MAJOR 0
#define SHIZEN_VERSION_MINOR 9
#define SHIZEN_VERSION_PATCH 1

#define SHIZEN_VERSION_NAME "ALPHA"

#include <stdbool.h>

#include "type.h"

typedef struct {
    const char *title;
    bool fullscreen;
    bool vsync;
    SHIZSize size;
} SHIZWindowSettings;

static const SHIZWindowSettings SHIZWindowSettingsDefault = {
    "SHIZEN",    /* title */
    false,       /* fullscreen */
    true,        /* vsync */
    { 320, 240 } /* preferred screen size */
};

/**
 * @brief Startup the SHIZEN engine core.
 *
 * Startup and initialize the SHIZEN engine core for use.
 *
 * Once initialized, SHIZEN has a graphics context and is ready to do things.
 *
 * @warning Initialization must occur before calling any other SHIZEN core functions.
 
 * @remark This function has no effect if SHIZEN was already initialized successfully.
 *
 * @param settings A SHIZWindowSettings object with values and flags
 *        that controls the resulting graphics context
 *
 * @return `true` if SHIZEN was initialized successfully, `false` otherwise
 */
bool shiz_startup(SHIZWindowSettings const settings);
/**
 * @brief Shutdown the SHIZEN engine core.
 *
 * Shutdown the SHIZEN engine core and clear any associated graphics context.
 *
 * If SHIZEN was successfully initialized, this function should be called before the game exits.
 *
 * @remark This function has no effect if SHIZEN was not initialized.
 *
 * @return `true` if SHIZEN was shutdown successfully, `false` otherwise
 */
bool shiz_shutdown(void);

/**
 * @brief Signal that SHIZEN should finish up.
 *
 * Signal that the SHIZEN graphics context should finish up and prepare to shutdown.
 *
 * This can be called at any time (e.g. during the rendering of a frame), to indicate that 
 * the game should shutdown when possible.
 */
void shiz_request_finish(void);
/**
 * @brief Determine whether SHIZEN should finish up.
 *
 * Determine whether SHIZEN has been signaled that it
 * should finish up and prepare to shutdown.
 *
 * This can occur in two ways: either
 *  1) the graphics context has been forced closed (e.g. by user closing the window), or
 *  2) the game has signaled that SHIZEN should finish up
 *
 * @return `true` if SHIZEN should finish up, `false` otherwise
 */
bool shiz_should_finish(void);

// load a resource and return its id - return invalid id (0) if loading failed
uint shiz_load(const char *filename);
// unload a resource - return true if unloading succeeded, false otherwise
bool shiz_unload(uint const resource_id);

SHIZSprite shiz_load_sprite(uint const resource_id);
SHIZSprite shiz_load_sprite_src(uint const resource_id, SHIZRect source);

SHIZSpriteFont shiz_load_sprite_font(SHIZSprite const sprite, SHIZSize const character, SHIZASCIITable const table);

/**
 * @brief Initiate a drawing/rendering context.
 *
 * Initiate a context for drawing and rendering to the graphics context.
 *
 * Additionally, clear the screen from the previous frame.
 */
void shiz_drawing_begin(void);
/**
 * @brief Draw a line.
 */
void shiz_draw_line(SHIZVector2 const from, SHIZVector2 const to, SHIZColor const color);
/**
 * @brief Draw a path.
 */
void shiz_draw_path(SHIZVector2 const points[], uint const count, SHIZColor const color);
/**
 * @brief Draw a rect.
 */
void shiz_draw_rect(SHIZRect const rect, SHIZColor const color);

void shiz_draw_sprite(SHIZSprite const sprite, SHIZVector2 const origin, SHIZSize const size);
void shiz_draw_sprite_ex(SHIZSprite const sprite, SHIZVector2 const origin, SHIZSize const size, SHIZVector2 const anchor, float const angle, SHIZColor const tint, bool const repeat);

void shiz_draw_sprite_text(SHIZSpriteFont const font, const char* text, SHIZVector2 const origin, SHIZSize const size, SHIZVector2 const scale, SHIZColor const tint);

/**
 * @brief Finish a drawing/rendering context.
 *
 * Finish the current drawing/rendering context and
 * present the current frame to the graphics context.
 */
void shiz_drawing_end(void);

#endif // engine_h
