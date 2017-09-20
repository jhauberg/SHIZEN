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

#include <SHIZEN/zlayer.h>

#include "internal.h"
#include "internal_type.h"

static
float const
z_layer__get_z_between(float value,
                       float min,
                       float max);

SHIZLayer const SHIZLayerTop = {
    .layer = SHIZLayerMax,
    .depth = SHIZLayerDepthMax
};

SHIZLayer const SHIZLayerBottom = {
    .layer = SHIZLayerMin,
    .depth = SHIZLayerDepthMin
};

static
float const
z_layer__get_z_between(float const value,
                       float const min,
                       float const max)
{
    return (value - min) / (max - min);
}

float const
z_layer__get_z(SHIZLayer const layer)
{
    // to provide a depth range for the top-most layer (e.g. 255),
    // we add an "additional" layer just above
    float const layer_max = SHIZLayerMax + 1;
    
    float const z = z_layer__get_z_between(layer.layer,
                                           SHIZLayerMin,
                                           layer_max);
    float const z_above = z_layer__get_z_between(layer.layer + 1,
                                                 SHIZLayerMin,
                                                 layer_max);
    
    float const depth_z = z_layer__get_z_between(layer.depth,
                                                 SHIZLayerDepthMin,
                                                 SHIZLayerDepthMax);
    
    return z_lerp(z, z_above, depth_z);
}
