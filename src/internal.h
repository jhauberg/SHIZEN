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

#ifndef internal_h
#define internal_h

#define SHIZEN_VERSION_MAJOR 0
#define SHIZEN_VERSION_MINOR 21
#define SHIZEN_VERSION_PATCH 0

#define SHIZEN_VERSION_NAME "ALPHA"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <SHIZEN/zint.h>

#ifdef SHIZ_DEBUG
f64
z_time__get_lag(void);
#endif

void
z_engine__present_frame(void);

static inline
void
z_str_to_upper(char * string)
{
    char * ptr;

    for (ptr = string; *ptr; ptr++) {
        *ptr = (char)toupper(*ptr);
    }
}

#endif // internal_h
