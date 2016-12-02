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

#include <SHIZEN/debug.h>

#include "res.h"
#include "io.h"

void shiz_debug_print_resources(void) {
#ifdef DEBUG
    shiz_res_debug_print_resources();
#else
    shiz_io_error("debug function called, but DEBUG not defined; consider ifdef'ing the call away");
#endif
}
