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

#include "shader.h" // z_gfx_*

#include "../internal.h" // GLuint, GLenum, GLchar, gl*

#include <stdio.h> // fprintf

GLuint
z_gfx__compile_shader(GLenum const type,
                      GLchar const * const source)
{
    GLuint const shader = glCreateShader(type);
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint param;
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);

    if (param != GL_TRUE) {
        GLchar log[4096];
        
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        
        fprintf(stderr, "compile error: %s", (char *)log);
        
        return 0;
    }
    
    return shader;
}

GLuint
z_gfx__link_program(GLuint const vs,
                    GLuint const fs)
{
    GLuint const program = glCreateProgram();
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    
    glLinkProgram(program);
    
    GLint param;
    
    glGetProgramiv(program, GL_LINK_STATUS, &param);
    
    if (param != GL_TRUE) {
        GLchar log[4096];
        
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        
        fprintf(stderr, "link error: %s", (char *)log);
        
        return 0;
    }
    
    return program;
}
