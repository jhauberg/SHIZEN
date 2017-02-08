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

SHIZVector2 const SHIZVector2Zero = { 0, 0 };
SHIZVector2 const SHIZVector2One = { 1, 1 };
SHIZVector3 const SHIZVector3Zero = { 0, 0, 0 };
SHIZVector3 const SHIZVector3One = { 1, 1, 1 };

SHIZColor const SHIZColorWhite = { 1, 1, 1, 1 };
SHIZColor const SHIZColorBlack = { 0, 0, 0, 1 };
SHIZColor const SHIZColorRed = { 1, 0, 0, 1 };
SHIZColor const SHIZColorGreen = { 0, 1, 0, 1 };
SHIZColor const SHIZColorBlue = { 0, 0, 1, 1 };
SHIZColor const SHIZColorYellow = { 1, 1, 0, 1 };

SHIZSize const SHIZSizeZero = { 0, 0 };

SHIZRect const SHIZRectEmpty = {
    .origin = { 0, 0 },
    .size = { 0, 0 }
};

SHIZVector2 const SHIZAnchorCenter = { 0.0f, 0.0f };
SHIZVector2 const SHIZAnchorTop = { 0.0f, 1.0f };
SHIZVector2 const SHIZAnchorTopLeft = { -1.0f, 1.0f };
SHIZVector2 const SHIZAnchorLeft = { -1.0f, 0.0f };
SHIZVector2 const SHIZAnchorBottomLeft = { -1.0f, -1.0f };
SHIZVector2 const SHIZAnchorBottom = { 0.0f, -1.0f };
SHIZVector2 const SHIZAnchorTopRight = { 1.0f, 1.0f };
SHIZVector2 const SHIZAnchorRight = { 1.0f, 0.0f };
SHIZVector2 const SHIZAnchorBottomRight = { 1.0f, -1.0f };

SHIZSprite const SHIZSpriteEmpty = {
    .resource_id = 0,
    .source = {
        .origin = { 0, 0 },
        .size = { 0, 0 }
    }
};

SHIZSize const SHIZSpriteSizeIntrinsic = { -1, -1 };

SHIZSpriteFontAttributes const SHIZSpriteFontAttributesDefault = {
    .scale = { 1, 1 },
    .wrap = SHIZSpriteFontWrapModeWord,
    .character_spread = SHIZSpriteFontSpreadNormal,
    .character_padding = SHIZSpriteFontNoPadding,
    .line_padding = SHIZSpriteFontNoPadding
};
