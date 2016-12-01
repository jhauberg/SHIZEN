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

#include <stdio.h>
#include <stdarg.h>

#include "io.h"

static const uint buffer_capacity = 256;
static char buffer[buffer_capacity];

void shiz_io_error(const char *format, ...) {
    va_list args;
    va_start(args, format); {
        vsnprintf(buffer, buffer_capacity, format, args);

        fprintf(stderr, "*** %s ***\n", buffer);
    }
    va_end(args);
}
