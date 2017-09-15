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

typedef bool (* shiz_io_image_loaded_handler)(int width, int height, int components, unsigned char * data);

void shiz_io_error(char const * format, ...);
void shiz_io_warning(char const * format, ...);
void shiz_io_error_context(char const * context, char const * format, ...);
void shiz_io_warning_context(char const * context, char const * format, ...);

bool shiz_io_load_image(char const * filename,
                        shiz_io_image_loaded_handler handler);
bool shiz_io_load_image_data(unsigned char const * buffer, unsigned int length,
                             shiz_io_image_loaded_handler handler);

#endif // io_h
