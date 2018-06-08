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

#pragma once

#include <stdbool.h> // bool
#include <stdint.h> // uint8_t, int16_t, uint32_t, int32_t

typedef bool (* z_io__load_image_handler)(int32_t width, int32_t height, int32_t components, uint8_t * data);
typedef bool (* z_io__load_sound_handler)(int32_t channels, int32_t sample_rate, int16_t * data, int32_t size);

void z_io__error(char const * format, ...);
void z_io__warning(char const * format, ...);

#ifdef SHIZ_DEBUG
void z_io__debug(char const * format, ...);
#endif

void z_io__error_context(char const * context, char const * format, ...);
void z_io__warning_context(char const * context, char const * format, ...);

bool z_io__load_image(char const * filename, z_io__load_image_handler);
bool z_io__load_image_data(uint8_t const * buffer, uint32_t length, z_io__load_image_handler);
bool z_io__load_sound(char const * filename, z_io__load_sound_handler);
