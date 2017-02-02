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

typedef enum SHIZViewportMode {
    SHIZViewportModeNormal,
    SHIZViewportModeLetterbox,
    SHIZViewportModePillarbox
} SHIZViewportMode;

typedef struct SHIZVertexPositionColor {
    SHIZVector3 position;
    SHIZColor color;
} SHIZVertexPositionColor;

typedef struct SHIZVertexPositionColorTexture {
    SHIZVector3 position;
    SHIZColor color;
    SHIZVector2 texture_coord;
    SHIZVector2 texture_coord_min;
    SHIZVector2 texture_coord_max;
} SHIZVertexPositionColorTexture;

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
 * @brief Render a textured quad.
 * 
 * Render a textured quad at a location, optionally rotated at an angle.
 *
 * @remark This function batches vertex data, and is only flushed when necessary (but at least
 *         once per frame).
 */
void shiz_gfx_render_quad(SHIZVertexPositionColorTexture const * restrict vertices,
                          SHIZVector3 const origin,
                          float const angle,
                          GLuint const texture_id);

void shiz_gfx_flush(void);

void shiz_gfx_clear(void);
void shiz_gfx_begin(void);
void shiz_gfx_end(void);

SHIZViewport shiz_gfx_get_viewport(void);

void shiz_gfx_set_viewport(SHIZViewport const viewport);

#ifdef DEBUG
unsigned int shiz_gfx_debug_get_draw_count(void);
unsigned int shiz_gfx_debug_get_frames_per_second(void);
unsigned int shiz_gfx_debug_get_frames_per_second_min(void);
unsigned int shiz_gfx_debug_get_frames_per_second_max(void);
unsigned int shiz_gfx_debug_get_frames_per_second_avg(void);
double shiz_gfx_debug_get_frame_time(void);
double shiz_gfx_debug_get_frame_time_avg(void);
#endif

#endif // gfx_h
