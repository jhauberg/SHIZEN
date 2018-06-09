#include <SHIZEN/ztype.h>

#include <stdbool.h> // false
#include <stdlib.h> // NULL

#include "internal.h" // SHIZVector3

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

SHIZSpriteSheet const SHIZSpriteSheetEmpty = {
    .rows = 0,
    .columns = 0,
    .resource = {
        .resource_id = 0,
        .source = {
            .origin = { 0, 0 },
            .size = { 0, 0 }
        }
    },
    .sprite_size = {
        .width = 0,
        .height = 0
    }
};

SHIZSpriteFontTable const SHIZSpriteFontTableEmpty = {
    .columns = 0,
    .rows = 0,
    .codepage = NULL
};

SHIZSpriteFont const SHIZSpriteFontEmpty = {
    .sprite = {
        .resource_id = 0,
        .source = {
            .origin = { 0, 0 },
            .size = { 0, 0 }
        }
    },
    .character = { 0, 0 },
    .table = {
        .columns = 0,
        .rows = 0,
        .codepage = NULL
    },
    .includes_whitespace = false
};

SHIZSpriteSize const SHIZSpriteSizeIntrinsic = {
    .target = { -1, -1 },
    .scale = { SHIZSpriteNoScale, SHIZSpriteNoScale }
};

SHIZSpriteFontAttributes const SHIZSpriteFontAttributesDefault = {
    .scale = { SHIZSpriteNoScale, SHIZSpriteNoScale },
    .wrap = SHIZSpriteFontWrapModeWord,
    .character_spread = SHIZSpriteFontSpreadNormal,
    .character_padding = SHIZSpriteFontNoPadding,
    .line_padding = SHIZSpriteFontNoPadding,
    .colors = NULL,
    .colors_count = 0
};
