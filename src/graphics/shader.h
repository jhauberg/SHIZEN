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

#include "../internal.h" // GLuint, GLenum, GLchar

GLuint z_gfx__compile_shader(GLenum type, GLchar const * source);
GLuint z_gfx__link_program(GLuint vs, GLuint fs);
