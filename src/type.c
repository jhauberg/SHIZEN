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

#include <SHIZEN/type.h>

const SHIZVector2 SHIZVector2Zero = { 0, 0 };
const SHIZVector2 SHIZVector2One = { 1, 1 };
const SHIZVector3 SHIZVector3Zero = { 0, 0, 0 };
const SHIZVector3 SHIZVector3One = { 1, 1, 1 };

const SHIZColor SHIZColorWhite = { 1, 1, 1, 1 };
const SHIZColor SHIZColorBlack = { 0, 0, 0, 1 };
const SHIZColor SHIZColorRed = { 1, 0, 0, 1 };
const SHIZColor SHIZColorGreen = { 0, 1, 0, 1 };
const SHIZColor SHIZColorBlue = { 0, 0, 1, 1 };

const SHIZSize SHIZSizeEmpty = { 0, 0 };
const SHIZRect SHIZRectEmpty = {
    { 0, 0 }, { 0, 0 }
};

const SHIZSprite SHIZSpriteEmpty = {
    0, { { 0, 0 }, { 0, 0 } }
};

const SHIZSize SHIZSpriteSizeIntrinsic = { -1, -1 };

const float SHIZSpriteNoAngle = 0;
const uint SHIZSpriteLayerDefault = 0;

const SHIZVector2 SHIZSpriteAnchorCenter = { 0.0f, 0.0f };
const SHIZVector2 SHIZSpriteAnchorTop = { 0.0f, 1.0f };
const SHIZVector2 SHIZSpriteAnchorTopLeft = { -1.0f, 1.0f };
const SHIZVector2 SHIZSpriteAnchorLeft = { -1.0f, 0.0f };
const SHIZVector2 SHIZSpriteAnchorBottomLeft = { -1.0f, -1.0f };
const SHIZVector2 SHIZSpriteAnchorBottom = { 0.0f, -1.0f };
const SHIZVector2 SHIZSpriteAnchorTopRight = { 1.0f, 1.0f };
const SHIZVector2 SHIZSpriteAnchorRight = { 1.0f, 0.0f };
const SHIZVector2 SHIZSpriteAnchorBottomRight = { 1.0f, -1.0f };

const SHIZSpriteFontAttributes SHIZSpriteFontAttributesDefault = {
    SHIZSpriteFontWrapModeWord, { 1, 1 },
    SHIZSpriteFontSpreadNormal,
    SHIZSpriteFontNoPadding, SHIZSpriteFontNoPadding
};
