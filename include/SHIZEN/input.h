////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2017 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#ifndef input_h
#define input_h

#include <stdbool.h>

typedef enum SHIZInput {
    SHIZInputUp, SHIZInputDown,
    SHIZInputLeft, SHIZInputRight,
    SHIZInputConfirm, SHIZInputEscape,
    SHIZInputAny
} SHIZInput;

#define SHIZInputMax (SHIZInputAny + 1)

void shiz_input_update(void);
bool shiz_input_down(SHIZInput const input);
bool shiz_input_pressed(SHIZInput const input);
bool shiz_input_released(SHIZInput const input);

#endif /* input_h */
