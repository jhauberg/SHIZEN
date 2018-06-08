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

#pragma once

#include <stdbool.h> // bool

typedef enum SHIZInput {
    SHIZInputUp,
    SHIZInputDown,
    SHIZInputLeft,
    SHIZInputRight,
    SHIZInputConfirm,
    SHIZInputEscape,
    SHIZInputSpace,
    SHIZInputX,
    SHIZInputZ,
    SHIZInputAny
} SHIZInput;

#define SHIZInputMax (SHIZInputAny + 1)

void z_input_update(void);

bool z_input_down(SHIZInput);
bool z_input_pressed(SHIZInput);
bool z_input_released(SHIZInput);
