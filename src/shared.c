#include <stdbool.h> // bool
#include <stdlib.h> // NULL

#include "internal.h" // SHIZGraphicsContext

SHIZGraphicsContext _graphics_context = {
    .window = NULL,
    .native_size = {
        .width = 0,
        .height = 0
    },
    .display_size = {
        .width = 0,
        .height = 0
    },
    .swap_interval = 0,
    .pixel_size = 1,
    .is_initialized = false,
    .is_focused = false,
    .should_finish = false
};

SHIZSprite _spr_white_1x1;
