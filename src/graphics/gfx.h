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

#pragma once

#include <stdbool.h>

#include "../internal.h"
#include "../viewport.h"

#include "../res.h"

/**
 * @brief Initialize the SHIZEN graphics module.
 *
 * Initialize and prepare the SHIZEN graphics module for use.
 *
 * @warning Initialization must occur before calling any other graphics
 *          functions.
 *
 * @param viewport
 *        The viewport that will be rendered into
 *
 * @return `true` if the graphics module was initialized successfully,
 *         `false` otherwise
 */
bool z_gfx__init(SHIZViewport viewport);

/**
 * @brief Kill any resources created by the graphics module.
 *
 * Kills and clears any resources/render objects allocated by the graphics
 * module.
 *
 * @return `true` if the graphics module was cleared successfully,
 *         `false` otherwise
 */
bool z_gfx__kill(void);

/**
 * @brief Render vertex data.
 *
 * Render pre-transformed vertex data by first allocating a buffer on the GPU,
 * then uploading vertex data to it and finally drawing primitives of the
 * specified type.
 *
 * @remark This function does not batch vertex data. Every call will result in
 * an additional buffer allocation and a draw call, and so it is not very
 * efficient. Take that into consideration before extended use.
 */
void z_gfx__render(GLenum mode, SHIZVertexPositionColor const * restrict vertices, uint32_t count);
void z_gfx__render_ex(GLenum const mode, SHIZVertexPositionColor const * restrict vertices, uint32_t count, SHIZVector3 origin, float angle);

/**
 * @brief Render a sprite; a textured quad.
 *
 * Render a sprite at a location, optionally rotated at an angle.
 *
 * @remark This function batches vertex data, and is only flushed when
 *         necessary (but at least once per frame).
 */
void z_gfx__render_sprite(SHIZVertexPositionColorTexture const * restrict vertices, SHIZVector3 origin, float angle, GLuint texture_id);

void z_gfx__begin(SHIZColor clear);

void z_gfx__end(void);
void z_gfx__flush(void);

bool z_gfx__create_texture(SHIZResourceImage *, int32_t width, int32_t height, int32_t components, uint8_t * data);
bool z_gfx__destroy_texture(SHIZResourceImage const *);
