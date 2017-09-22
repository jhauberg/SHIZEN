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

#include <math.h> // roundf

#include "viewport.h"

#include "io.h"

SHIZViewport const SHIZViewportDefault = {
    .framebuffer = {
        .width = 0,
        .height = 0
    },
    .resolution = {
        .width = 0,
        .height = 0
    },
    .scale = 1,
    .is_fullscreen = false
};

static
void
z_viewport__apply_boxing_if_necessary(void);

static
void
z_viewport__determine_operating_resolution(void);

static
void
z_viewport__determine_mode(SHIZViewportMode * mode,
                           f32 * width,
                           f32 * height);

// set to false to let viewport fit framebuffer (pixels will be stretched)
#define SHIZViewportEnableBoxing true

static SHIZViewport _viewport;
static SHIZSize _viewport_offset;

SHIZViewport
z_viewport__get()
{
    return _viewport;
}

SHIZSize
z_viewport__get_offset()
{
    return _viewport_offset;
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
                                  _viewport.is_fullscreen ? "fullscreen" : "windowed");
        }
}

static
void
z_viewport__apply_boxing_if_necessary()
{
    _viewport_offset = SHIZSizeZero;
    
    if (SHIZViewportEnableBoxing) {
        SHIZViewportMode mode;
        
        f32 adjusted_width;
        f32 adjusted_height;
        
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
                           f32 * const width,
                           f32 * const height)
{
    f32 const screen_aspect_ratio = _viewport.resolution.width / _viewport.resolution.height;
    f32 const framebuffer_aspect_ratio = _viewport.framebuffer.width / _viewport.framebuffer.height;
    
    f32 adjusted_width = _viewport.framebuffer.width;
    f32 adjusted_height = _viewport.framebuffer.height;
    
    *mode = SHIZViewportModeNormal;
    
    if (screen_aspect_ratio > framebuffer_aspect_ratio ||
        framebuffer_aspect_ratio > screen_aspect_ratio) {
        f32 const targetAspectRatio = screen_aspect_ratio;
        
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
