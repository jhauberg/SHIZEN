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

typedef bool (* shiz_io_image_loaded_handler)(int, int, int, unsigned char *);

void shiz_io_error(const char * const format, ...);
void shiz_io_error_context(const char * const context, const char * const format, ...);
void shiz_io_warning(const char * const format, ...);
void shiz_io_warning_context(const char * const context, const char * const format, ...);

bool shiz_io_load_image(const char * const filename, shiz_io_image_loaded_handler const handler);
bool shiz_io_load_image_data(const unsigned char * const buffer, uint const length, shiz_io_image_loaded_handler const handler);

#endif // io_h
