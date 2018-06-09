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
