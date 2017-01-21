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
#define SHIZEN_VERSION_MINOR 14
#define SHIZEN_VERSION_PATCH 0

#define SHIZEN_VERSION_NAME "ALPHA"

#include <stdbool.h>

#include "type.h"
#include "draw.h"

typedef struct SHIZWindowSettings SHIZWindowSettings;

/**
 * Provides settings and flags for the creation of the window.
 */
struct SHIZWindowSettings {
    /** The name of the window */
    const char *title;
    /** Determines whether the window should be fullscreen */
    bool fullscreen;
    /** Determines whether v-sync should be enabled */
    bool vsync;
    /** The size of the window */
    SHIZSize size;
};

/**
 * @brief Default window settings.
 *
 * Default window settings provides a 320x240 non-fullscreen window with v-sync enabled.
 */
extern const SHIZWindowSettings SHIZWindowSettingsDefault;

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
 * @param settings 
 *        A SHIZWindowSettings object with values and flags that
 *        controls the resulting graphics context
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

void shiz_ticking_begin(void);
bool shiz_tick(uint const frequency);
float shiz_ticking_end(void);

double shiz_get_time(void);
double shiz_get_tick_rate(void);

/**
 * @brief Load a resource.
 *
 * @return A resource id if the resource was loaded successfully, `0` otherwise
 */
uint shiz_load(const char * const filename);
/**
 * @brief Unload a resource.
 *
 * @return `true` if the resource was unloaded successfully, `false` otherwise
 */
bool shiz_unload(uint const resource_id);

SHIZSprite shiz_load_sprite(const char * const filename);
SHIZSprite shiz_load_sprite_src(const char * const filename, SHIZRect const source);

SHIZSprite shiz_get_sprite(uint const resource_id);
SHIZSprite shiz_get_sprite_src(uint const resource_id, SHIZRect const source);

SHIZSpriteFont shiz_load_sprite_font(const char * const filename,
                                     SHIZSize const character);
SHIZSpriteFont shiz_load_sprite_font_ex(const char * const filename,
                                        SHIZSize const character,
                                        SHIZSpriteFontTable const table);

SHIZSpriteFont shiz_get_sprite_font(SHIZSprite const sprite,
                                    SHIZSize const character);
SHIZSpriteFont shiz_get_sprite_font_ex(SHIZSprite const sprite,
                                       SHIZSize const character,
                                       SHIZSpriteFontTable const table);

#endif // engine_h
