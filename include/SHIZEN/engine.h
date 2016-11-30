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
#define SHIZEN_VERSION_MINOR 1
#define SHIZEN_VERSION_PATCH 0

#include <stdbool.h>

#include "type.h"

/**
 * @brief Initialize the SHIZEN engine core.
 *
 * Initialize and prepare the SHIZEN engine core for use.
 *
 * Once initialized, SHIZEN has a graphics context and is ready to do things.
 *
 * @warning Initialization must occur before calling any other SHIZEN core functions.
 *
 * @remark This function has no effect if SHIZEN was already initialized successfully.
 *
 * @return `true` if SHIZEN was initialized correctly, `false` otherwise
 */
bool shiz_init(void);
/**
 * @brief Shutdown the SHIZEN engine core.
 *
 * Shutdown the SHIZEN engine core and clear any associated graphics context.
 *
 * If SHIZEN was successfully initialized, this function should be called before the game exits.
 *
 * @remark This function has no effect if SHIZEN was not initialized.
 *
 * @return `true` if SHIZEN was shutdown correctly, `false` otherwise
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
 * Determine whether SHIZEN has been signaled that it should finish up and prepare to shutdown.
 *
 * This can occur in two ways: either
 *  1) the graphics context has been forced closed (e.g. by user closing the window), or
 *  2) the game has signaled that SHIZEN should finish up
 *
 * @return `true` if SHIZEN should finish up, `false` otherwise
 */
bool shiz_should_finish(void);

void shiz_drawing_begin(void);
void shiz_draw_line(SHIZPoint const from, SHIZPoint const to, SHIZColor const color);
void shiz_draw_path(SHIZPoint const points[], uint const count, SHIZColor const color);
void shiz_drawing_end(void);

#endif // engine_h
