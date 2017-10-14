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

#include "gfx.h"

#include "../transform.h"
#include "../res.h"
#include "../io.h"
#include "../white.1x1.h"

#include "shader.h"
#include "spritebatch.h"
#include "immediate.h"

#ifdef SHIZ_DEBUG
 #include "../debug/debug.h"
#endif

#include <stdbool.h>

#include <SHIZEN/zloader.h>

#ifdef SHIZ_DEBUG
static
void
z_gfx__process_errors(void);
#endif

extern SHIZSprite _spr_white_1x1;

static
bool
z_gfx__init_post(void);

static
bool
z_gfx__kill_post(void);

static
void
z_gfx__render_post(void);

static
bool
z_gfx__load_default_texture(void);

#define VERTEX_COUNT_PER_FRAME 4

typedef struct SHIZGFXPost {
    SHIZRenderObject render;
    GLuint texture_id;
    GLuint framebuffer;
    GLuint renderbuffer;
} SHIZGFXPost;

static SHIZGFXPost _post;

bool
z_gfx__init(SHIZViewport const viewport)
{
    z_viewport__set(viewport);

    if (!z_gfx__init_immediate()) {
        z_io__error_context("GFX", "Could not initialize immediate renderer");
        
        return false;
    }

    if (!z_gfx__init_spritebatch()) {
        z_io__error_context("GFX", "Could not initialize spritebatch renderer");
        
        return false;
    }

    if (!z_gfx__init_post()) {
        z_io__error_context("GFX", "Could not initialize post renderer");
        
        return false;
    }
    
    if (!z_gfx__load_default_texture()) {
        z_io__error_context("GFX", "Could not load default texture");
        
        return false;
    }

    return true;
}

bool
z_gfx__kill()
{
    if (!z_unload(_spr_white_1x1.resource_id)) {
        return false;
    }
    
    _spr_white_1x1.resource_id = SHIZResourceInvalid;
    _spr_white_1x1.source = SHIZRectEmpty;
    
    if (!z_gfx__kill_immediate()) {
        return false;
    }

    if (!z_gfx__kill_spritebatch()) {
        return false;
    }

    if (!z_gfx__kill_post()) {
        return false;
    }

    return true;
}

void
z_gfx__begin(SHIZColor const clear)
{
#ifdef SHIZ_DEBUG
    z_debug__reset_draw_count();
#endif

    z_gfx__spritebatch_reset();

    SHIZViewport const viewport = z_viewport__get();
    
    GLint const x = 0;
    GLint const y = 0;
    
    GLsizei const width = (GLsizei)viewport.resolution.width;
    GLsizei const height = (GLsizei)viewport.resolution.height;
    
    glBindFramebuffer(GL_FRAMEBUFFER, _post.framebuffer); {
        glViewport(x, y, width, height);

        glClearColor(clear.r, clear.g, clear.b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void
z_gfx__end()
{
    z_gfx__flush();

    SHIZRect const clip = z_viewport__get_clip();
    
    GLint const x = (GLint)clip.origin.x;
    GLint const y = (GLint)clip.origin.y;
    
    GLsizei const width = (GLsizei)clip.size.width;
    GLsizei const height = (GLsizei)clip.size.height;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0); {
        glViewport(x, y, width, height);
        
        // note that we don't need to clear this framebuffer, as we're expecting to overwrite
        // every pixel every frame anyway (with the opaque texture of the post framebuffer)
        // however; if we wanted to apply color to the letter/pillar-boxed bars,
        // we could do that here by clearing the color buffer

        z_gfx__render_post();
    }

#ifdef SHIZ_DEBUG
    z_gfx__process_errors();
    
    z_debug__update_frame_stats();
#endif
}

void
z_gfx__flush()
{
    z_gfx__spritebatch_flush();
}

void
z_gfx__render(GLenum const mode,
              SHIZVertexPositionColor const * restrict const vertices,
              u32 const count)
{
    z_gfx__render_ex(mode, vertices, count, SHIZVector3Zero, 0);
}

void
z_gfx__render_ex(GLenum const mode,
                 SHIZVertexPositionColor const * restrict const vertices,
                 u32 const count,
                 SHIZVector3 const origin,
                 f32 const angle)
{
    z_gfx__render_immediate(mode, vertices, count, origin, angle);
}

void
z_gfx__render_sprite(SHIZVertexPositionColorTexture const * restrict const vertices,
                     SHIZVector3 const origin,
                     f32 const angle,
                     GLuint const texture_id)
{
    z_gfx__add_sprite(vertices, origin, angle, texture_id);
}

bool
z_gfx__create_texture(SHIZResourceImage * const resource,
                      int const width,
                      int const height,
                      int const components,
                      unsigned char * const data)
{
    if (resource == NULL) {
        return false;
    }
    
    glGenTextures(1, &resource->texture_id);
    glBindTexture(GL_TEXTURE_2D, resource->texture_id); {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        if (components == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (components == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

bool
z_gfx__destroy_texture(SHIZResourceImage const * const resource)
{
    if (resource == NULL) {
        return false;
    }
    
    glDeleteTextures(1, &resource->texture_id);
    
    return true;
}

static
bool
z_gfx__init_post()
{
    char const * const vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec2 vertex_texture_coord;\n"
    "out vec2 texture_coord;\n"
    "void main() {\n"
    "    gl_Position = vec4(vertex_position, 1);\n"
    "    texture_coord = vertex_texture_coord.st;\n"
    "}\n";

    char const * const fragment_shader =
    "#version 330 core\n"
    "in vec2 texture_coord;\n"
    "uniform sampler2D sampler;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    vec4 sampled_color = texture(sampler, texture_coord.st);\n"
    "    fragment_color = sampled_color;\n"
    "}\n";

    GLuint const vs = z_gfx__compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint const fs = z_gfx__compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    if (!vs && !fs) {
        return false;
    }

    _post.render.program = z_gfx__link_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!_post.render.program) {
        return false;
    }

    SHIZViewport const viewport = z_viewport__get();
    SHIZColor const clear_color = SHIZColorBlack;
    
    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1);
    // clear depth should be set once and not changed afterwards
    glClearDepth(1.0);
    
    GLsizei const w = (GLsizei)viewport.resolution.width;
    GLsizei const h = (GLsizei)viewport.resolution.height;
    
    glGenFramebuffers(1, &_post.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _post.framebuffer); {
        glGenTextures(1, &_post.texture_id);
        glBindTexture(GL_TEXTURE_2D, _post.texture_id); {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         w, h,
                         0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glGenRenderbuffers(1, &_post.renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, _post.renderbuffer); {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
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

    static SHIZVertexPositionTexture const vertices[VERTEX_COUNT_PER_FRAME] = {
        { .position = { -1, -1, 0 }, .texture_coord = { 0, 0 } },
        { .position = { -1,  1, 0 }, .texture_coord = { 0, 1 } },
        { .position = {  1, -1, 0 }, .texture_coord = { 1, 0 } },
        { .position = {  1,  1, 0 }, .texture_coord = { 1, 1 } },
    };
    
    glGenBuffers(1, &_post.render.vbo);
    glGenVertexArrays(1, &_post.render.vao);
    
    glBindVertexArray(_post.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _post.render.vbo); {
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                         GL_STATIC_DRAW);
            
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

static
void
z_gfx__render_post()
{
    glUseProgram(_post.render.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _post.texture_id); {
        glBindVertexArray(_post.render.vao); {
            glBindBuffer(GL_ARRAY_BUFFER, _post.render.vbo); {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_COUNT_PER_FRAME);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

static
bool
z_gfx__kill_post()
{
    glDeleteFramebuffers(1, &_post.framebuffer);
    glDeleteRenderbuffers(1, &_post.renderbuffer);
    glDeleteTextures(1, &_post.texture_id);
    glDeleteProgram(_post.render.program);
    glDeleteVertexArrays(1, &_post.render.vao);
    glDeleteBuffers(1, &_post.render.vbo);

    return true;
}

static
bool
z_gfx__load_default_texture()
{
    u8 const white_resource_id = z_res__load_data(SHIZResourceTypeImage,
                                                  WHITE_1x1,
                                                  WHITE_1x1_SIZE);
    
    if (white_resource_id == SHIZResourceInvalid) {
        return false;
    }
    
    _spr_white_1x1 = z_load_sprite_from(white_resource_id);
    
    return true;
}

#ifdef SHIZ_DEBUG
static
void
z_gfx__process_errors()
{
    GLenum error;
    
    while ((error = glGetError()) != GL_NO_ERROR) {
        z_io__error_context("OPENGL", "%d", error);
    }
}
#endif
