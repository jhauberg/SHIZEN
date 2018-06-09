#pragma once

#include "internal.h" // SHIZVector3, mat4x4
#include "viewport.h" // SHIZViewport

void z_transform__project_ortho(mat4x4 world, mat4x4 model, SHIZViewport viewport);
void z_transform__translate_rotate_scale(mat4x4 model, SHIZVector3 translation, float angle, float scale);
