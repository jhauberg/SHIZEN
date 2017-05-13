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

#include "viewport.h"

/**
 * @brief Initialize the SHIZEN graphics module.
 *
 * Initialize and prepare the SHIZEN graphics module for use.
 *
 * @warning Initialization must occur before calling any other graphics functions.
 *
 * @param viewport
 *        The viewport that will be rendered into
 *
 * @return `true` if the graphics module was initialized successfully, `false` otherwise
 */
bool shiz_gfx_init(SHIZViewport const viewport);
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
 * Render pre-transformed vertex data by first allocating a buffer on the GPU, 
 * then uploading vertex data to it and finally drawing primitives of the specified type.
 *
 * @remark This function does not batch vertex data. Every call will result in an 
 * additional buffer allocation and a draw call, and so it is not very efficient. 
 * Take that into consideration before extended use.
 */
void shiz_gfx_render(GLenum const mode,
                     SHIZVertexPositionColor const * restrict vertices,
                     unsigned int const count);
void shiz_gfx_render_ex(GLenum const mode,
                        SHIZVertexPositionColor const * restrict vertices,
                        unsigned int const count,
                        SHIZVector3 const origin,
                        float const angle);

/**
 * @brief Render a sprite; a textured quad.
 *
 * Render a sprite at a location, optionally rotated at an angle.
 *
 * @remark This function batches vertex data, and is only flushed when necessary (but at least
 *         once per frame).
 */
void shiz_gfx_render_sprite(SHIZVertexPositionColorTexture const * restrict vertices,
                            SHIZVector3 const origin,
                            float const angle,
                            GLuint const texture_id);

void shiz_gfx_begin(SHIZColor const clear);
void shiz_gfx_end(void);

void shiz_gfx_flush(void);

#endif // gfx_h
