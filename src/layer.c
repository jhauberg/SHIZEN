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

#include <SHIZEN/layer.h>

SHIZLayer const SHIZLayerTop = {
    .layer = SHIZLayerMax,
    .depth = SHIZLayerDepthMax
};

SHIZLayer const SHIZLayerBottom = {
    .layer = SHIZLayerMin,
    .depth = SHIZLayerDepthMin
};
