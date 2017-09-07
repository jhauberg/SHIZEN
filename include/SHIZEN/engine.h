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

#include <stdbool.h>

#include "type.h"
#include "draw.h"
#include "time.h"
#include "input.h"

/**
 * Provides settings and flags for the creation of the window.
 */
typedef struct SHIZWindowSettings {
    /** The title of the window */
    const char * title;
    /** A description of the game that is printed to the log */
    const char * description;
    /** Determines whether the window should be fullscreen initially */
    bool fullscreen;
    /** Determines whether v-sync should be enabled */
    bool vsync;
    /** The size of the display within the window in pixels; the actual window 
      * size is determined by pixel-size and may be larger than the display */
    SHIZSize size;
    /** The size of each pixel; defaults to 1; a higher pixel size results in 
      * a larger window */
    unsigned int pixel_size;
} SHIZWindowSettings;

/**
 * @brief Default window settings.
 *
 * Default window settings provides a 320x240 non-fullscreen window with v-sync enabled.
 */
extern SHIZWindowSettings const SHIZWindowSettingsDefault;

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

/**
 * @brief Return the display size within the window.
 */
SHIZSize shiz_get_display_size(void);

/**
 * @brief Load a resource.
 *
 * @return A resource id if the resource was loaded successfully, `0` otherwise
 */
unsigned int shiz_load(const char * const filename);
/**
 * @brief Unload a resource.
 *
 * @return `true` if the resource was unloaded successfully, `false` otherwise
 */
bool shiz_unload(unsigned int const resource_id);

SHIZSprite shiz_load_sprite(const char * const filename);
SHIZSprite shiz_load_sprite_src(const char * const filename, SHIZRect const source);

SHIZSprite shiz_get_sprite(unsigned int const resource_id);
SHIZSprite shiz_get_sprite_src(unsigned int const resource_id, SHIZRect const source);

SHIZSpriteSheet shiz_load_sprite_sheet(const char * const filename, SHIZSize const sprite_size);
SHIZSpriteSheet shiz_load_sprite_sheet_src(const char * const filename,
                                           SHIZSize const sprite_size,
                                           SHIZRect const source);

SHIZSpriteSheet shiz_get_sprite_sheet(SHIZSprite const resource, SHIZSize const sprite_size);
SHIZSpriteSheet shiz_get_sprite_sheet_src(SHIZSprite const resource,
                                          SHIZSize const sprite_size,
                                          SHIZRect const source);

SHIZSprite shiz_get_sprite_index(SHIZSpriteSheet const spritesheet, unsigned int const index);
SHIZSprite shiz_get_sprite_colrow(SHIZSpriteSheet const spritesheet,
                                  unsigned int const column,
                                  unsigned int const row);

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
