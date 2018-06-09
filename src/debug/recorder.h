#pragma once

#include <SHIZEN/ztype.h> // SHIZSize

#include <stdint.h> // uint8_t
#include <stdbool.h> // bool

bool z_recorder__init(void);
bool z_recorder__kill(void);

void z_recorder__setup(SHIZSize resolution,
                       uint8_t hz);

void z_recorder__capture(void);

bool z_recorder__start(void);
bool z_recorder__stop(void);

bool z_recorder_is_recording(void);
