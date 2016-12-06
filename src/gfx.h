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
 * @param framebuffer_size The size (in pixels) of the backbuffer (note that this is not
 *                         necesarilly the same size as the actual window)
 * @param pixel_scale The ratio between the size of the framebuffer and the window
 *
 * @return `true` if the graphics module was initialized successfully, `false` otherwise
 */
bool shiz_gfx_init(SHIZSize const framebuffer_size, float const pixel_scale);
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
void shiz_gfx_render_quad(SHIZVertexPositionColorTexture const *vertices, GLuint texture_id);

void shiz_gfx_clear(void);
void shiz_gfx_begin(void);
void shiz_gfx_end(void);

void shiz_gfx_set_framebuffer_size(SHIZSize const framebuffer_size, float const pixel_scale);

#ifdef DEBUG
uint shiz_gfx_debug_get_draw_count(void);
#endif

#endif // gfx_h
