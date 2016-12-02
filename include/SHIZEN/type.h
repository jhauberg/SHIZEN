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

#ifndef type_h
#define type_h

typedef struct {
    float x, y;
} SHIZPoint;

static const SHIZPoint SHIZPointZero = { 0, 0 };
static const SHIZPoint SHIZPointOne = { 1, 1 };

static inline SHIZPoint SHIZPointMake(float const x, float const y) {
    SHIZPoint point = {
        x, y
    };

    return point;
}

typedef struct {
    SHIZPoint center;
    float width;
    float height;
} SHIZRect;

static const SHIZRect SHIZRectEmpty = { { 0, 0 }, 0, 0 };

static inline SHIZRect SHIZRectMake(SHIZPoint center, float const width, float const height) {
    SHIZRect rect = {
        center, width, height
    };

    return rect;
}

typedef struct {
    float r, g, b;
    float alpha;
} SHIZColor;

static const SHIZColor SHIZColorWhite = { 1, 1, 1, 1 };
static const SHIZColor SHIZColorBlack = { 0, 0, 0, 1 };
static const SHIZColor SHIZColorRed = { 1, 0, 0, 1 };

static inline SHIZColor SHIZColorMake(float const r, float const g, float const b, float const alpha) {
    SHIZColor color = {
        r, g, b, alpha
    };

    return color;
}

#endif // type_h
