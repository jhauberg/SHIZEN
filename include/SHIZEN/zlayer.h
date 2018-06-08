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

#pragma once

#include <stdint.h> // uint8_t, UINT8_MAX

typedef struct SHIZLayer {
    uint8_t depth;
    uint8_t layer;
} SHIZLayer;

extern SHIZLayer const SHIZLayerTop; // max
extern SHIZLayer const SHIZLayerBottom; // min

#define SHIZLayerMin 0
#define SHIZLayerMax UINT8_MAX

#define SHIZLayerDepthMin 0
#define SHIZLayerDepthMax UINT8_MAX

#define SHIZLayerDefault SHIZLayerBottom

static inline
SHIZLayer const
SHIZLayeredAtDepth(uint8_t const layer,
                   uint8_t const depth)
{
    SHIZLayer result;

    result.layer = layer;
    result.depth = depth;

    return result;
}

static inline
SHIZLayer const
SHIZLayered(uint8_t const layer)
{
    return SHIZLayeredAtDepth(layer, SHIZLayerDefault.depth);
}

static inline
SHIZLayer const
SHIZLayeredBelow(SHIZLayer const layer)
{
    SHIZLayer layer_below = layer;

    if (layer.depth > SHIZLayerDepthMin) {
        layer_below.depth = layer.depth - 1;
    } else if (layer.layer > SHIZLayerMin) {
        layer_below.layer = layer.layer - 1;
    }

    return layer_below;
}

static inline
SHIZLayer const
SHIZLayeredAbove(SHIZLayer const layer)
{
    SHIZLayer layer_above = layer;

    if (layer.depth < SHIZLayerDepthMax) {
        layer_above.depth = layer.depth + 1;
    } else if (layer.layer < SHIZLayerMax) {
        layer_above.layer = layer.layer + 1;
    }

    return layer_above;
}
