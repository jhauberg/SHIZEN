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

#ifndef gfx_h
#define gfx_h

#include "internal.h"

/**
 * @brief Initialize the SHIZEN graphics module.
 *
 * Initialize and prepare the SHIZEN graphics module for use.
 *
 * @warning Initialization must occur before calling any other graphics functions.
 *
 * @return `true` if the graphics module was initialized successfully, `false` otherwise
 */
bool shiz_gfx_init(void);
/**
 * @brief Kill any resources created by the graphics module.
 *
 * Kills and clears any resources/render objects allocated by the graphics module.
 *
 * @return `true` if the graphics module was cleared successfully, `false` otherwise
 */
bool shiz_gfx_kill(void);

/**
 * @brief Render vertex data.
 *
 * Render vertex data by first allocating a buffer on the GPU, then uploading vertex data to it
 * and finally drawing primitives of the specified type.
 *
 * @remark This function does not batch vertex data. Every call will result in an 
 * additional buffer allocation and a draw call, and so it is not very efficient. 
 * Take that into consideration before extended use.
 */
void shiz_gfx_render(GLenum const mode, SHIZVertexPositionColor const *vertices, uint const count);

void shiz_gfx_clear(void);

#endif // gfx_h
