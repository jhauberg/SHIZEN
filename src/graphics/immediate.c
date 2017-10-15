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

#include "immediate.h"

#include "shader.h"
#include "viewport.h"
#include "transform.h"

#ifdef SHIZ_DEBUG
 #include "../debug/debug.h"
 #include "../debug/profiler.h"
#endif

static
void
z_gfx__immediate_state(bool enable);

static SHIZRenderObject _renderer;

bool
z_gfx__init_immediate()
{
    char const * const vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec4 vertex_color;\n"
    "uniform mat4 transform;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "    gl_Position = transform * vec4(vertex_position, 1);\n"
    "    color = vertex_color;\n"
    "}\n";
    
    char const * const fragment_shader =
    "#version 330 core\n"
    "in vec4 color;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    fragment_color = color;\n"
    "}\n";
    
    GLuint const vs = z_gfx__compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint const fs = z_gfx__compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
    
    if (!vs && !fs) {
        return false;
    }
    
    _renderer.program = z_gfx__link_program(vs, fs);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    if (!_renderer.program) {
        return false;
    }
    
    glGenBuffers(1, &_renderer.vbo);
    glGenVertexArrays(1, &_renderer.vao);
    
    glBindVertexArray(_renderer.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _renderer.vbo); {
            glVertexAttribPointer(0 /* position location */,
                                  3 /* number of position components per vertex */,
                                  GL_FLOAT, GL_FALSE /* values are not normalized */,
                                  sizeof(SHIZVertexPositionColor) /* offset to reach next vertex */,
                                  0 /* position component is the first, so no offset */);
            glEnableVertexAttribArray(0);
            
            glVertexAttribPointer(1 /* color location */,
                                  4 /* number of color components per vertex */,
                                  GL_FLOAT, GL_FALSE,
                                  sizeof(SHIZVertexPositionColor),
                                  (GLvoid*)(sizeof(SHIZVector3)) /* offset to reach color component */);
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    return true;
}

void
z_gfx__render_immediate(GLenum const mode,
                        SHIZVertexPositionColor const * restrict const vertices,
                        u32 const count,
                        SHIZVector3 const origin,
                        f32 const angle)
{
    mat4x4 model;
    mat4x4_identity(model);
    
    z_transform__translate_rotate_scale(model, origin, angle, 1.0f);
    
    mat4x4 transform;
    mat4x4_identity(transform);
    
    // todo: optimization; in many cases we don't have to keep building the projection matrix
    //                     because it only changes when the viewport changes- which is probably not every frame
    z_transform__project_ortho(transform, model, z_viewport__get());
    
    z_gfx__immediate_state(true);
    
    glUseProgram(_renderer.program);
    glUniformMatrix4fv(glGetUniformLocation(_renderer.program, "transform"), 1, GL_FALSE, *transform);
    glBindVertexArray(_renderer.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _renderer.vbo); {
            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(SHIZVertexPositionColor) * count,
                         vertices,
                         GL_DYNAMIC_DRAW);
            glDrawArrays(mode, 0, (GLsizei)count /* count of indices; not count of lines; i.e. 1 line = 2 vertices/indices */);
#ifdef SHIZ_DEBUG
            z_profiler__increment_draw_count(1);
#endif
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);
    
    z_gfx__immediate_state(false);
    
#ifdef SHIZ_DEBUG
    if (origin.x == 0 && origin.y == 0 && origin.z == 0 && count > 0) {
        z_debug__add_event_draw(SHIZDebugEventNamePrimitive, vertices[0].position);
    } else {
        z_debug__add_event_draw(SHIZDebugEventNamePrimitive, origin);
    }
#endif
}

bool
z_gfx__kill_immediate()
{
    glDeleteProgram(_renderer.program);
    glDeleteVertexArrays(1, &_renderer.vao);
    glDeleteBuffers(1, &_renderer.vbo);
    
    return true;
}

static
void
z_gfx__immediate_state(bool const enable)
{
    if (enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
    }
}
