#pragma once

#include <stdbool.h> // bool
#include <stdint.h> // uint8_t, int16_t, int32_t

#include "res.h" // SHIZResourceSound

bool z_mixer__init(void);
bool z_mixer__kill(void);

bool z_mixer__create_sound(SHIZResourceSound * resource,
                           int32_t channels,
                           int32_t sample_rate,
                           int16_t * data,
                           int32_t size);

bool z_mixer__destroy_sound(SHIZResourceSound const * resource);
