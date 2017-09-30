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

#ifndef shizen_h
#define shizen_h

#include <stdbool.h>

#include "ztype.h"
#include "zrand.h"
#include "ztime.h"
#include "zloader.h"
#include "zinput.h"
#include "zdraw.h"

/**
 * @brief Provides settings and flags for the creation of a window.
 */
typedef struct SHIZWindowSettings {
    /** The size of the display within the window in pixels; the actual window
     * size is determined by pixel-size and may be larger than the display */
    SHIZSize size;
    /** The title of the window */
    char const * title;
    /** A description of the game that is printed to the log */
    char const * description;
    /** The size of each pixel; defaults to 1; a higher pixel size results in
     * a larger window */
    u8 pixel_size;
    /** Determines whether the window should be fullscreen initially */
    bool fullscreen;
    /** Determines whether v-sync should be enabled */
    bool vsync;
    u8 _pad[5];
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
bool
z_startup(SHIZWindowSettings settings);

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
bool
z_shutdown(void);

/**
 * @brief Signal that SHIZEN should finish up.
 *
 * Signal that the SHIZEN graphics context should finish up and prepare to shutdown.
 *
 * This can be called at any time (e.g. during the rendering of a frame), to indicate that 
 * the game should shutdown when possible.
 */
void
z_request_finish(void);

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
bool
z_should_finish(void);

/**
 * @brief Return the display size within the window.
 */
SHIZSize
z_get_display_size(void);

#endif // shizen_h
