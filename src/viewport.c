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

#include "viewport.h"

#include "io.h"

static void _shiz_apply_viewport_boxing_if_necessary(void);
static void _shiz_determine_operating_resolution(void);
static void _shiz_determine_viewport_mode(SHIZViewportMode * mode,
                                          float * width,
                                          float * height);

// set to false to let viewport fit framebuffer (pixels will be stretched)
static bool const enable_boxing_if_necessary = true;

static SHIZViewport _viewport;
static SHIZSize _viewport_offset;

SHIZViewport
shiz_get_viewport()
{
    return _viewport;
}

SHIZSize
shiz_get_viewport_offset()
{
    return _viewport_offset;
}

void
shiz_set_viewport(SHIZViewport const viewport)
{
    if (viewport.framebuffer.width == _viewport.framebuffer.width ||
        viewport.framebuffer.height == _viewport.framebuffer.height) {
        return;
    }
    
    _viewport = viewport;
    
    _shiz_determine_operating_resolution();
    _shiz_apply_viewport_boxing_if_necessary();
}

static
void
_shiz_determine_operating_resolution()
{
    if ((_viewport.resolution.width < _viewport.framebuffer.width ||
         _viewport.resolution.width > _viewport.framebuffer.width) ||
        (_viewport.resolution.height < _viewport.framebuffer.height ||
         _viewport.resolution.height > _viewport.framebuffer.height)) {
            shiz_io_warning_context("GFX", "Operating resolution is %.0fx%.0f @ %.0fx%.0f@%.0fx (%s)",
                                    _viewport.resolution.width, _viewport.resolution.height,
                                    _viewport.framebuffer.width, _viewport.framebuffer.height,
                                    _viewport.scale,
                                    _viewport.is_fullscreen ? "fullscreen" : "windowed");
        }
}

static
void
_shiz_apply_viewport_boxing_if_necessary()
{
    _viewport_offset = SHIZSizeZero;
    
    if (enable_boxing_if_necessary) {
        SHIZViewportMode mode;
        
        float adjusted_width;
        float adjusted_height;
        
        _shiz_determine_viewport_mode(&mode, &adjusted_width, &adjusted_height);
        
        if (mode != SHIZViewportModeNormal) {
            _viewport_offset = SHIZSizeMake(_viewport.framebuffer.width - adjusted_width,
                                            _viewport.framebuffer.height - adjusted_height);
            
            shiz_io_warning_context("GFX", "Aspect ratio mismatch between the operating resolution and the framebuffer; enabling %s",
                                    (mode == SHIZViewportModeLetterbox ? "letterboxing" : "pillarboxing"));
        }
    }
}

static
void
_shiz_determine_viewport_mode(SHIZViewportMode * const mode,
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
