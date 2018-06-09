#pragma once

#include "../internal.h" // GLuint, GLenum, GLchar

GLuint z_gfx__compile_shader(GLenum type, GLchar const * source);
GLuint z_gfx__link_program(GLuint vs, GLuint fs);
