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

#include "shader.h"
#include "transform.h"
#include "spritebatch.h"
#include "io.h"

#ifdef SHIZ_DEBUG
static const char * const _shiz_debug_event_primitive = "\xDB";
#endif

static void _shiz_gfx_post_state(bool const enable);

static bool _shiz_gfx_init_post(void);
static bool _shiz_gfx_kill_post(void);
static void _shiz_gfx_render_post(void);

static bool _shiz_gfx_init_primitive(void);
static bool _shiz_gfx_kill_primitive(void);
static void _shiz_gfx_render_primitive(GLenum const mode,
                                       SHIZVertexPositionColor const * restrict vertices,
                                       unsigned int const count,
                                       SHIZVector3 const origin,
                                       float const angle);

static void _shiz_gfx_primitive_state(bool const enable);

static unsigned int const post_vertex_count = 4;

typedef struct SHIZGFXPost {
    GLuint texture_id;
    GLuint framebuffer;
    GLuint renderbuffer;
    SHIZRenderObject render;
} SHIZGFXPost;

typedef struct SHIZGFXPrimitive {
    SHIZRenderObject render;
} SHIZGFXPrimitive;

static SHIZGFXPrimitive _primitive;
static SHIZGFXPost _post;

bool
shiz_gfx_init(SHIZViewport const viewport)
{
    shiz_set_viewport(viewport);

    if (!_shiz_gfx_init_primitive()) {
        shiz_io_error_context("GFX", "Could not initialize primitive");
        
        return false;
    }

    if (!shiz_gfx_init_spritebatch()) {
        shiz_io_error_context("GFX", "Could not initialize spritebatch");
        
        return false;
    }

    if (!_shiz_gfx_init_post()) {
        shiz_io_error_context("GFX", "Could not initialize post");
        
        return false;
    }

    return true;
}

bool
shiz_gfx_kill()
{
    if (!_shiz_gfx_kill_primitive()) {
        return false;
    }

    if (!shiz_gfx_kill_spritebatch()) {
        return false;
    }

    if (!_shiz_gfx_kill_post()) {
        return false;
    }

    return true;
}

void
shiz_gfx_begin()
{
#ifdef SHIZ_DEBUG
    shiz_debug_reset_draw_count();
#endif

    shiz_gfx_spritebatch_reset();

    SHIZViewport const viewport = shiz_get_viewport();

    glBindFramebuffer(GL_FRAMEBUFFER, _post.framebuffer); {
        glViewport(0, 0,
                   viewport.resolution.width,
                   viewport.resolution.height);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void
shiz_gfx_end()
{
    shiz_gfx_flush();

    SHIZViewport const viewport = shiz_get_viewport();
    SHIZSize const viewport_offset = shiz_get_viewport_offset();

    glBindFramebuffer(GL_FRAMEBUFFER, 0); {
        glViewport(viewport_offset.width / 2,
                   viewport_offset.height / 2,
                   viewport.framebuffer.width - viewport_offset.width,
                   viewport.framebuffer.height - viewport_offset.height);

        // note that we don't need to clear this framebuffer, as we're expecting to overwrite
        // every pixel every frame anyway (with the opaque texture of the post framebuffer)
        // however; if we wanted to apply color to the letter/pillar-boxed bars,
        // we could do that here by clearing the color buffer

        _shiz_gfx_render_post();
    }

#ifdef SHIZ_DEBUG
    shiz_debug_update_frame_stats();
#endif
}

void
shiz_gfx_flush()
{
    shiz_gfx_spritebatch_flush();
}

void
shiz_gfx_render(GLenum const mode,
                SHIZVertexPositionColor const * restrict vertices,
                unsigned int const count)
{
    shiz_gfx_render_ex(mode, vertices, count, SHIZVector3Zero, 0);
}

void
shiz_gfx_render_ex(GLenum const mode,
                   SHIZVertexPositionColor const * restrict vertices,
                   unsigned int const count,
                   SHIZVector3 const origin,
                   float const angle)
{
    _shiz_gfx_render_primitive(mode, vertices, count, origin, angle);
}

void
shiz_gfx_render_sprite(SHIZVertexPositionColorTexture const * restrict vertices,
                       SHIZVector3 const origin,
                       float const angle,
                       GLuint const texture_id)
{
    shiz_gfx_add_sprite(vertices, origin, angle, texture_id);
}

static void
_shiz_gfx_post_state(bool const enable)
{
    if (enable) {

    }
}

static bool
_shiz_gfx_init_post()
{
    const char * vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec2 vertex_texture_coord;\n"
    "out vec2 texture_coord;\n"
    "void main() {\n"
    "    gl_Position = vec4(vertex_position, 1);\n"
    "    texture_coord = vertex_texture_coord.st;\n"
    "}\n";

    const char * fragment_shader =
    "#version 330 core\n"
    "in vec2 texture_coord;\n"
    "uniform sampler2D sampler;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    vec4 sampled_color = texture(sampler, texture_coord.st);\n"
    "    fragment_color = sampled_color;\n"
    "}\n";

    GLuint const vs = shiz_gfx_compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint const fs = shiz_gfx_compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    if (!vs && !fs) {
        return false;
    }

    _post.render.program = shiz_gfx_link_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!_post.render.program) {
        return false;
    }

    SHIZViewport const viewport = shiz_get_viewport();
    SHIZColor const clear_color = SHIZColorBlack;
    
    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1);
    // clear depth should be set once and not changed afterwards
    glClearDepth(1.0);
    
    GLsizei const texture_width = viewport.resolution.width;
    GLsizei const texture_height = viewport.resolution.height;
    
    glGenFramebuffers(1, &_post.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _post.framebuffer); {
        glGenTextures(1, &_post.texture_id);
        glBindTexture(GL_TEXTURE_2D, _post.texture_id); {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         texture_width, texture_height,
                         0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glGenRenderbuffers(1, &_post.renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, _post.renderbuffer); {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture_width, texture_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _post.renderbuffer);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _post.texture_id, 0);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        GLenum const draw_buffers[1] = {
            GL_COLOR_ATTACHMENT0
        };

        glDrawBuffers(1, draw_buffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            return false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    static const SHIZVertexPositionTexture vertices[post_vertex_count] = {
        { .position = { -1, -1, 0 }, .texture_coord = { 0, 0 } },
        { .position = { -1,  1, 0 }, .texture_coord = { 0, 1 } },
        { .position = {  1, -1, 0 }, .texture_coord = { 1, 0 } },
        { .position = {  1,  1, 0 }, .texture_coord = { 1, 1 } },
    };
    
    glGenBuffers(1, &_post.render.vbo);
    glGenVertexArrays(1, &_post.render.vao);
    
    glBindVertexArray(_post.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _post.render.vbo); {
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            
            glVertexAttribPointer(0 /* position location */,
                                  3 /* number of position components per vertex */,
                                  GL_FLOAT, GL_FALSE /* values are not normalized */,
                                  sizeof(SHIZVertexPositionTexture) /* offset to reach next vertex */,
                                  0 /* position component is the first, so no offset */);
            glEnableVertexAttribArray(0);
            
            glVertexAttribPointer(1 /* texture coord location */,
                                  2 /* number of color components per vertex */,
                                  GL_FLOAT, GL_FALSE,
                                  sizeof(SHIZVertexPositionTexture),
                                  (GLvoid*)(sizeof(SHIZVector3)) /* offset to reach color component */);
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    return true;
}

static void
_shiz_gfx_render_post()
{
    _shiz_gfx_post_state(true);

    glUseProgram(_post.render.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _post.texture_id); {
        glBindVertexArray(_post.render.vao); {
            glBindBuffer(GL_ARRAY_BUFFER, _post.render.vbo); {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, post_vertex_count);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    _shiz_gfx_post_state(false);
}

static bool
_shiz_gfx_kill_post()
{
    glDeleteFramebuffers(1, &_post.framebuffer);
    glDeleteRenderbuffers(1, &_post.renderbuffer);
    glDeleteTextures(1, &_post.texture_id);
    glDeleteProgram(_post.render.program);
    glDeleteVertexArrays(1, &_post.render.vao);
    glDeleteBuffers(1, &_post.render.vbo);

    return true;
}

static bool
_shiz_gfx_init_primitive()
{
    const char * vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec4 vertex_color;\n"
    "uniform mat4 transform;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "    gl_Position = transform * vec4(vertex_position, 1);\n"
    "    color = vertex_color;\n"
    "}\n";
    
    const char * fragment_shader =
    "#version 330 core\n"
    "in vec4 color;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    fragment_color = color;\n"
    "}\n";
    
    GLuint const vs = shiz_gfx_compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint const fs = shiz_gfx_compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
    
    if (!vs && !fs) {
        return false;
    }
    
    _primitive.render.program = shiz_gfx_link_program(vs, fs);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    if (!_primitive.render.program) {
        return false;
    }
    
    glGenBuffers(1, &_primitive.render.vbo);
    glGenVertexArrays(1, &_primitive.render.vao);
    
    glBindVertexArray(_primitive.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _primitive.render.vbo); {
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

static void
_shiz_gfx_render_primitive(GLenum const mode,
                           SHIZVertexPositionColor const * restrict vertices,
                           unsigned int const count,
                           SHIZVector3 const origin,
                           float const angle)
{
    mat4x4 model;
    mat4x4_identity(model);
    
    shiz_transform_translate_rotate_scale(model, origin, angle, 1);
    
    mat4x4 transform;
    mat4x4_identity(transform);
    
    shiz_transform_project_ortho(transform, model, shiz_get_viewport());
    
    _shiz_gfx_primitive_state(true);
    
    glUseProgram(_primitive.render.program);
    glUniformMatrix4fv(glGetUniformLocation(_primitive.render.program, "transform"), 1, GL_FALSE, *transform);
    glBindVertexArray(_primitive.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _primitive.render.vbo); {
            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(SHIZVertexPositionColor) * count,
                         vertices,
                         GL_DYNAMIC_DRAW);
            glDrawArrays(mode, 0, count /* count of indices; not count of lines; i.e. 1 line = 2 vertices/indices */);
#ifdef SHIZ_DEBUG
            shiz_debug_increment_draw_count(1);
#endif
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);
    
    _shiz_gfx_primitive_state(false);
    
#ifdef SHIZ_DEBUG
    if (origin.x == 0 && origin.y == 0 && origin.z == 0 && count > 0) {
        shiz_debug_add_event_draw(_shiz_debug_event_primitive, vertices[0].position);
    } else {
        shiz_debug_add_event_draw(_shiz_debug_event_primitive, origin);
    }
#endif
}

static bool
_shiz_gfx_kill_primitive()
{
    glDeleteProgram(_primitive.render.program);
    glDeleteVertexArrays(1, &_primitive.render.vao);
    glDeleteBuffers(1, &_primitive.render.vbo);
    
    return true;
}

static void
_shiz_gfx_primitive_state(bool const enable)
{
    if (enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}
