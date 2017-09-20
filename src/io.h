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

#ifndef io_h
#define io_h

#include <stdbool.h>

typedef bool (* z_io__load_image_handler)(int width, int height, int components, unsigned char * data);

void
z_io__error(char const * format, ...);

void
z_io__warning(char const * format, ...);

void
z_io__error_context(char const * context, char const * format, ...);

void
z_io__warning_context(char const * context, char const * format, ...);

bool
z_io__load_image(char const * filename,
                 z_io__load_image_handler handler);

bool
z_io__load_image_data(unsigned char const * buffer,
                      unsigned int length,
                      z_io__load_image_handler handler);

#endif // io_h
