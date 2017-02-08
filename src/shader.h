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

#include "internal.h"

GLuint shiz_gfx_compile_shader(GLenum const type, const GLchar * source);
GLuint shiz_gfx_link_program(GLuint const vs, GLuint const fs);

#endif // shader_h
