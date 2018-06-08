////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2017 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#include "viewport.h" // SHIZViewport, z_viewport_*

#include <math.h> // roundf

#include "internal.h" // SHIZGraphicsContext
#include "io.h" // z_io_*

SHIZViewport const SHIZViewportDefault = {
    .framebuffer = {
        .width = 0,
        .height = 0
    },
    .resolution = {
        .width = 0,
        .height = 0
    },
    .scale = 1
};

extern SHIZGraphicsContext const _graphics_context;

static void z_viewport__apply_boxing_if_necessary(void);

static void z_viewport__determine_operating_resolution(void);
static void z_viewport__determine_mode(SHIZViewportMode *, float * width, float * height);

// set to false to let viewport fit framebuffer (pixels will be stretched)
#define SHIZViewportEnableBoxing true

static SHIZViewport _viewport;
static SHIZSize _viewport_offset;

SHIZViewport
z_viewport__get()
{
    return _viewport;
}

SHIZRect
z_viewport__get_clip()
{
    float const x = _viewport_offset.width / 2;
    float const y = _viewport_offset.height / 2;
    
    float const width = _viewport.framebuffer.width - _viewport_offset.width;
    float const height = _viewport.framebuffer.height - _viewport_offset.height;
    
    return SHIZRectMake(SHIZVector2Make(x, y),
                        SHIZSizeMake(width, height));
}

void
z_viewport__set(SHIZViewport const viewport)
{
    if (viewport.framebuffer.width == _viewport.framebuffer.width ||
        viewport.framebuffer.height == _viewport.framebuffer.height) {
        return;
    }
    
    _viewport = viewport;
    
    z_viewport__determine_operating_resolution();
    z_viewport__apply_boxing_if_necessary();
}

static
void
z_viewport__determine_operating_resolution()
{
    if ((_viewport.resolution.width < _viewport.framebuffer.width ||
         _viewport.resolution.width > _viewport.framebuffer.width) ||
        (_viewport.resolution.height < _viewport.framebuffer.height ||
         _viewport.resolution.height > _viewport.framebuffer.height)) {
            z_io__warning_context("GFX", "Operating resolution is %.0fx%.0f @ %.0fx%.0f@%.0fx (%s)",
                                  _viewport.resolution.width, _viewport.resolution.height,
                                  _viewport.framebuffer.width, _viewport.framebuffer.height,
                                  _viewport.scale,
                                  _graphics_context.is_fullscreen ? "fullscreen" : "windowed");
        }
}

static
void
z_viewport__apply_boxing_if_necessary()
{
    _viewport_offset = SHIZSizeZero;
    
    if (SHIZViewportEnableBoxing) {
        SHIZViewportMode mode;
        
        float adjusted_width;
        float adjusted_height;
        
        z_viewport__determine_mode(&mode, &adjusted_width, &adjusted_height);
        
        if (mode != SHIZViewportModeNormal) {
            _viewport_offset = SHIZSizeMake(_viewport.framebuffer.width - adjusted_width,
                                            _viewport.framebuffer.height - adjusted_height);
            
            z_io__warning_context("GFX", "Aspect ratio mismatch between the operating resolution and the framebuffer; enabling %s",
                                  (mode == SHIZViewportModeLetterbox ? "letterboxing" : "pillarboxing"));
        }
    }
}

static
void
z_viewport__determine_mode(SHIZViewportMode * const mode,
                           float * const width,
                           float * const height)
{
    float const screen_aspect_ratio = _viewport.resolution.width / _viewport.resolution.height;
    float const framebuffer_aspect_ratio = _viewport.framebuffer.width / _viewport.framebuffer.height;
    
    float adjusted_width = _viewport.framebuffer.width;
    float adjusted_height = _viewport.framebuffer.height;
    
    *mode = SHIZViewportModeNormal;
    
    if (screen_aspect_ratio > framebuffer_aspect_ratio ||
        framebuffer_aspect_ratio > screen_aspect_ratio) {
        float const targetAspectRatio = screen_aspect_ratio;
        
        // letterbox (horizontal bars)
        adjusted_height = roundf(adjusted_width / targetAspectRatio);
        
        *mode = SHIZViewportModeLetterbox;
        
        if (adjusted_height > _viewport.framebuffer.height) {
            // pillarbox (vertical bars)
            adjusted_height = _viewport.framebuffer.height;
            adjusted_width = roundf(adjusted_height * targetAspectRatio);
            
            *mode = SHIZViewportModePillarbox;
        }
    }
    
    *width = adjusted_width;
    *height = adjusted_height;
}
