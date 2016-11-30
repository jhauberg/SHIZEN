////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2016 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#include <stdbool.h>
#include <stdio.h>

#include "gfx.h"

static GLuint _shiz_gfx_compile_shader(GLenum const type, const GLchar *source);
static GLuint _shiz_gfx_link_program(GLuint const vert, GLuint const frag);

static SHIZRenderData render;

bool shiz_gfx_init() {
    const char *vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec2 vertex_position;\n"
    "layout (location = 1) in vec4 vertex_color;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "    gl_Position = vec4(vertex_position, 0, 1);\n"
    "    color = vertex_color;\n"
    "}\n";

    const char *fragment_shader =
    "#version 330 core\n"
    "in vec4 color;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    fragment_color = color;\n"
    "}\n";

    GLuint vs = _shiz_gfx_compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint fs = _shiz_gfx_compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    if (!vs && !fs) {
        return false;
    }

    render.program = _shiz_gfx_link_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!render.program) {
        return false;
    }

    glGenBuffers(1, &render.vbo);
    glGenVertexArrays(1, &render.vao);

    glBindVertexArray(render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, render.vbo); {
            glVertexAttribPointer(0 /* position location */,
                                  2 /* number of position components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE /* values are not normalized */,
                                  sizeof(SHIZVertexPositionColor) /* offset to reach next vertex */,
                                  0 /* position component is the first, so no offset */);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1 /* color location */,
                                  4 /* number of color components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  sizeof(SHIZVertexPositionColor),
                                  (GLvoid*)(sizeof(SHIZPoint)) /* offset to reach color component */);
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    return true;
}

bool shiz_gfx_kill() {
    glDeleteProgram(render.program);
    glDeleteVertexArrays(1, &render.vao);
    glDeleteBuffers(1, &render.vbo);

    return true;
}

void shiz_gfx_render(GLenum const mode, SHIZVertexPositionColor const *vertices, uint const count) {
    glUseProgram(render.program);
    glBindVertexArray(render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, render.vbo); {
            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(SHIZVertexPositionColor) * count /* sizeof(vertices) won't work here, because the array is passed as a pointer; but we can assume the type of vertex, so we can find the size manually ("sizeof(vertices[0]) * count" would work too) */,
                         vertices,
                         GL_DYNAMIC_DRAW /* because we never know how many lines will be rendered */);
            glDrawArrays(mode, 0, count /* count of vertices; not count of lines; i.e. 1 line = 2 vertices */);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

static GLuint _shiz_gfx_compile_shader(GLenum const type, const GLchar *source) {
    GLuint const shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint param;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);

    if (!param) {
        GLchar log[4096];

        glGetShaderInfoLog(shader, sizeof(log), NULL, log);

        fprintf(stderr, "shader compile error: %s\n", (char *) log);

        return 0;
    }

    return shader;
}

static GLuint _shiz_gfx_link_program(GLuint const vs, GLuint const fs) {
    GLuint const program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint param;

    glGetProgramiv(program, GL_LINK_STATUS, &param);

    if (!param) {
        GLchar log[4096];

        glGetProgramInfoLog(program, sizeof(log), NULL, log);

        fprintf(stderr, "shader program link error: link: %s\n", (char *) log);
        
        return 0;
    }
    
    return program;
}
