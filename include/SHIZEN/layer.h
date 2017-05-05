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

#ifndef layer_h
#define layer_h

typedef struct SHIZLayer {
    unsigned short depth: 16;
    unsigned short layer: 8;
} SHIZLayer;

extern SHIZLayer const SHIZLayerTop; // max
extern SHIZLayer const SHIZLayerBottom; // min

#define SHIZLayerMin 0
#define SHIZLayerMax 255 // UCHAR_MAX

#define SHIZLayerDepthMin 0
#define SHIZLayerDepthMax 65535 // USHRT_MAX

#define SHIZLayerDefault SHIZLayerBottom

static inline SHIZLayer const
SHIZLayeredWithDepth(unsigned char const layer,
                     unsigned short const depth) {
    SHIZLayer result;

    result.layer = layer;
    result.depth = depth;

    return result;
}

static inline SHIZLayer const
SHIZLayered(unsigned char const layer) {
    return SHIZLayeredWithDepth(layer, SHIZLayerDefault.depth);
}

static inline SHIZLayer const
SHIZLayeredBelow(SHIZLayer const layer) {
    SHIZLayer layer_below = layer;

    if (layer.depth > SHIZLayerDepthMin) {
        layer_below.depth = layer.depth - 1;
    } else if (layer.layer > SHIZLayerMin) {
        layer_below.layer = layer.layer - 1;
    }

    return layer_below;
}

static inline SHIZLayer const
SHIZLayeredAbove(SHIZLayer const layer) {
    SHIZLayer layer_above = layer;

    if (layer.depth < SHIZLayerDepthMax) {
        layer_above.depth = layer.depth + 1;
    } else if (layer.layer < SHIZLayerMax) {
        layer_above.layer = layer.layer + 1;
    }

    return layer_above;
}

#endif // layer_h
