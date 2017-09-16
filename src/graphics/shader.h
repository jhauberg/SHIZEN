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

#ifndef shader_h
#define shader_h

#include "internal_type.h"

GLuint shiz_gfx_compile_shader(GLenum type, GLchar const * source);
GLuint shiz_gfx_link_program(GLuint vs, GLuint fs);

#endif // shader_h
