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

#include <linmath.h>

#include "gfx.h"
#include "io.h"

#ifdef DEBUG
static void _shiz_gfx_debug_reset_draw_count(void);
static void _shiz_gfx_debug_increment_draw_count(uint amount);
#endif

static GLuint _shiz_gfx_compile_shader(GLenum const type, const GLchar *source);
static GLuint _shiz_gfx_link_program(GLuint const vert, GLuint const frag);

static void _shiz_gfx_apply_viewport_boxing_if_necessary(void);
static void _shiz_gfx_determine_viewport_mode(SHIZViewportMode *mode);
static void _shiz_gfx_determine_operating_resolution(void);

static bool _shiz_gfx_init_basic(void);
static bool _shiz_gfx_kill_basic(void);

static bool _shiz_gfx_init_batch(void);
static bool _shiz_gfx_kill_batch(void);

static void _shiz_gfx_batch_flush();
static uint _shiz_gfx_batch_count = 0;
static GLuint _shiz_gfx_batch_texture_id = 0;

static uint const max_batch = 32; /* flush when reaching this limit */
static uint const batch_vertex_count_per_quad = 2 * 3; /* 2 triangles per batched quad = 6 vertices  */
static uint const batch_vertex_count = max_batch * batch_vertex_count_per_quad;

static SHIZVertexPositionColorTexture batch_vertex_buffer[batch_vertex_count]; /* statically allocated buffer */

static SHIZRenderData basic_render;
static SHIZRenderData batch_render;

static SHIZViewport _viewport;

static bool const enable_boxing_if_necessary = true; // false to let viewport fit framebuffer (pixels will be stretched)

bool shiz_gfx_init(SHIZViewport const viewport) {
    shiz_gfx_set_viewport(viewport);

    if (!_shiz_gfx_init_basic()) {
        return false;
    }

    if (!_shiz_gfx_init_batch()) {
        return false;
    }

    return true;
}

bool shiz_gfx_kill() {
    if (!_shiz_gfx_kill_basic()) {
        return false;
    }

    if (!_shiz_gfx_kill_batch()) {
        return false;
    }

    return true;
}

static bool _shiz_gfx_init_basic() {
    const char *vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec4 vertex_color;\n"
    "uniform mat4 transform_mvp;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "    gl_Position = transform_mvp * vec4(vertex_position, 1);\n"
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
    
    basic_render.program = _shiz_gfx_link_program(vs, fs);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    if (!basic_render.program) {
        return false;
    }
    
    glGenBuffers(1, &basic_render.vbo);
    glGenVertexArrays(1, &basic_render.vao);
    
    glBindVertexArray(basic_render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, basic_render.vbo); {
            glVertexAttribPointer(0 /* position location */,
                                  3 /* number of position components per vertex */,
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
                                  (GLvoid*)(sizeof(SHIZVector3)) /* offset to reach color component */);
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    
    return true;
}

static bool _shiz_gfx_kill_basic() {
    glDeleteProgram(basic_render.program);
    glDeleteVertexArrays(1, &basic_render.vao);
    glDeleteBuffers(1, &basic_render.vbo);
    
    return true;
}

static bool _shiz_gfx_init_batch() {
    const char *vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec4 vertex_color;\n"
    "layout (location = 2) in vec2 vertex_texture_coord;\n"
    "layout (location = 3) in vec2 vertex_texture_coord_min;\n"
    "layout (location = 4) in vec2 vertex_texture_coord_max;\n"
    "uniform mat4 transform_mvp;\n"
    "out vec2 texture_coord;\n"
    "out vec2 texture_coord_min;\n"
    "out vec2 texture_coord_max;\n"
    "out vec4 tint_color;\n"
    "void main() {\n"
    "    gl_Position = transform_mvp * vec4(vertex_position, 1);\n"
    "    texture_coord = vertex_texture_coord.st;\n"
    "    texture_coord_min = vertex_texture_coord_min.xy;\n"
    "    texture_coord_max = vertex_texture_coord_max.xy;\n"
    "    tint_color = vertex_color;\n"
    "}\n";

    const char *fragment_shader =
    "#version 330 core\n"
    "in vec2 texture_coord;\n"
    "in vec2 texture_coord_min;\n"
    "in vec2 texture_coord_max;\n"
    "in vec4 tint_color;\n"
    "uniform int enable_additive_tint;\n"
    "uniform sampler2D sampler;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    float s = texture_coord.s;\n"
    "    if (texture_coord_min.x > 0 && texture_coord_max.x > 0) {\n"
    "        s = mod(mod(s, texture_coord_min.x), "
    "                       texture_coord_max.x) + texture_coord_min.x;\n"
    "    } else if (texture_coord_max.x > 0) {\n"
    "        s = mod(s, texture_coord_max.x) + texture_coord_min.x;\n"
    "    }\n"
    "    float t = texture_coord.t;\n"
    "    if (texture_coord_min.y > 0 && texture_coord_max.y > 0) {\n"
    "        t = mod(mod(t, texture_coord_min.y),"
    "                       texture_coord_max.y) + texture_coord_min.y;\n"
    "    } else if (texture_coord_max.y > 0) {\n"
    "        t = mod(t, texture_coord_max.y) + texture_coord_min.y;\n"
    "    }\n"
    "    vec2 repeated_texture_coord = vec2(s, t);\n"
    "    vec4 sampled_color = texture(sampler, repeated_texture_coord.st);\n"
    "    if (enable_additive_tint != 0) {\n"
    "        fragment_color = (sampled_color + vec4(tint_color.rgb, 0)) * tint_color.a;\n"
    "    } else {\n"
    "        fragment_color = sampled_color * tint_color;\n"
    "    }\n"
    "}\n";

    GLuint vs = _shiz_gfx_compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint fs = _shiz_gfx_compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    if (!vs && !fs) {
        return false;
    }

    batch_render.program = _shiz_gfx_link_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!batch_render.program) {
        return false;
    }
    
    glGenBuffers(1, &batch_render.vbo);
    glGenVertexArrays(1, &batch_render.vao);

    glBindVertexArray(batch_render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, batch_render.vbo); {
            int const stride = sizeof(SHIZVertexPositionColorTexture);

            glBufferData(GL_ARRAY_BUFFER,
                         batch_vertex_count * stride,
                         NULL /* we're just allocating the space initially- there's no vertex data yet */,
                         GL_DYNAMIC_DRAW /* we'll be updating this buffer regularly */);

            glVertexAttribPointer(0 /* position location */,
                                  3 /* number of position components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE /* values are not normalized */,
                                  stride /* offset to reach next vertex */,
                                  0 /* position component is the first, so no offset */);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1 /* color location */,
                                  4 /* number of color components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  (GLvoid*)(sizeof(SHIZVector3)) /* offset to reach color component */);
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2 /* texture coord location */,
                                  2 /* number of coord components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  (GLvoid*)(sizeof(SHIZVector3) + sizeof(SHIZColor)) /* offset to reach texture coord component */);
            glEnableVertexAttribArray(2);

            glVertexAttribPointer(3 /* texture coord scale location */,
                                  2 /* number of scale components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  (GLvoid*)(sizeof(SHIZVector3) + sizeof(SHIZColor) + sizeof(SHIZVector2)) /* offset to reach texture coord min component */);
            glEnableVertexAttribArray(3);

            glVertexAttribPointer(4 /* texture coord scale location */,
                                  2 /* number of scale components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  (GLvoid*)(sizeof(SHIZVector3) + sizeof(SHIZColor) + sizeof(SHIZVector2) + sizeof(SHIZVector2)) /* offset to reach texture coord max component */);
            glEnableVertexAttribArray(4);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    return true;
}

static bool _shiz_gfx_kill_batch() {
    glDeleteProgram(batch_render.program);
    glDeleteVertexArrays(1, &batch_render.vao);
    glDeleteBuffers(1, &batch_render.vbo);

    return true;
}

void shiz_gfx_clear() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shiz_gfx_begin() {
    glViewport(_viewport.aspect_ratio_offset.width / 2,
               _viewport.aspect_ratio_offset.height / 2,
               _viewport.framebuffer.width - _viewport.aspect_ratio_offset.width,
               _viewport.framebuffer.height - _viewport.aspect_ratio_offset.height);
    
#ifdef DEBUG
    _shiz_gfx_debug_reset_draw_count();
#endif
}

void shiz_gfx_end() {
    if (_shiz_gfx_batch_count > 0) {
        _shiz_gfx_batch_flush();
    }

    _shiz_gfx_batch_texture_id = 0;
}

static void mat4x4_model_view_projection(mat4x4 mvp) {
    mat4x4 model;
    mat4x4_identity(model);
    mat4x4 view;
    mat4x4_identity(view);
    mat4x4 projection;
    mat4x4_ortho(projection,
                 0, _viewport.screen.width,
                 0, _viewport.screen.height, -1, 1);

    mat4x4 view_model;
    mat4x4_mul(view_model, view, model);

    mat4x4_mul(mvp, projection, view_model);
}

void shiz_gfx_render(GLenum const mode, SHIZVertexPositionColor const *vertices, uint const count) {
    mat4x4 mvp;
    mat4x4_model_view_projection(mvp);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(basic_render.program);
    glUniformMatrix4fv(glGetUniformLocation(basic_render.program, "transform_mvp"), 1, GL_FALSE, *mvp);
    glBindVertexArray(basic_render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, basic_render.vbo); {
            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(SHIZVertexPositionColor) * count /* sizeof(vertices) won't work here, because the array is passed as a pointer */,
                         vertices,
                         GL_DYNAMIC_DRAW);
            glDrawArrays(mode, 0, count /* count of indices; not count of lines; i.e. 1 line = 2 vertices/indices */);
#ifdef DEBUG
            _shiz_gfx_debug_increment_draw_count(1);
#endif
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
}

void shiz_gfx_render_quad(SHIZVertexPositionColorTexture const *vertices, GLuint texture_id) {
    // todo: any way of doing bounds checking to ensure 6 vertices are supplied?
    // todo: if calls were sorted by texture, we could optimize to fewer flushes
    if (_shiz_gfx_batch_texture_id != 0 && /* dont flush if texture is not set yet */
        _shiz_gfx_batch_texture_id != texture_id) {
        _shiz_gfx_batch_flush();
    }

    _shiz_gfx_batch_texture_id = texture_id;

    if (_shiz_gfx_batch_count + 1 > max_batch) {
        _shiz_gfx_batch_flush();
    }

    uint const offset = _shiz_gfx_batch_count * batch_vertex_count_per_quad;

    // todo: memcpy probably faster
    for (uint i = 0; i < batch_vertex_count_per_quad; i++) {
        batch_vertex_buffer[offset + i] = vertices[i];
    }

    _shiz_gfx_batch_count += 1;
}

static void _shiz_gfx_batch_flush() {
    if (_shiz_gfx_batch_count == 0) {
        return;
    }

    mat4x4 mvp;
    mat4x4_model_view_projection(mvp);

    glClearDepth(1.0);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(batch_render.program);
    // todo: a way to provide this flag; problem is that it affects the entire batch
    glUniform1i(glGetUniformLocation(batch_render.program, "enable_additive_tint"), false);
    glUniformMatrix4fv(glGetUniformLocation(batch_render.program, "transform_mvp"), 1, GL_FALSE, *mvp);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _shiz_gfx_batch_texture_id); {
        glBindVertexArray(batch_render.vao); {
            glBindBuffer(GL_ARRAY_BUFFER, batch_render.vbo); {
                uint const index_count = _shiz_gfx_batch_count * batch_vertex_count_per_quad;

                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                index_count * sizeof(SHIZVertexPositionColorTexture),
                                batch_vertex_buffer);
                glDrawArrays(GL_TRIANGLES, 0, index_count);
#ifdef DEBUG
                _shiz_gfx_debug_increment_draw_count(1);
#endif
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    _shiz_gfx_batch_count = 0;
}

void shiz_gfx_set_viewport(SHIZViewport const viewport) {
    _viewport = viewport;

    _shiz_gfx_determine_operating_resolution();
    _shiz_gfx_apply_viewport_boxing_if_necessary();
}

static void _shiz_gfx_determine_operating_resolution() {
    if ((_viewport.screen.width < _viewport.framebuffer.width || _viewport.screen.width > _viewport.framebuffer.width) ||
        (_viewport.screen.height < _viewport.framebuffer.height || _viewport.screen.height > _viewport.framebuffer.height)) {
        shiz_io_warning_context("GFX", "Operating resolution is %.0fx%.0f @ %.0fx%.0f@%.0fx",
                                _viewport.screen.width, _viewport.screen.height,
                                _viewport.framebuffer.width, _viewport.framebuffer.height,
                                _viewport.scale);
    }
}

static void _shiz_gfx_apply_viewport_boxing_if_necessary() {
    _viewport.aspect_ratio_offset = SHIZSizeEmpty;

    if (enable_boxing_if_necessary) {
        SHIZViewportMode mode;

        _shiz_gfx_determine_viewport_mode(&mode);

        if (mode != SHIZViewportModeNormal) {
            shiz_io_warning_context("GFX", "Aspect ratio mismatch between the operating resolution and the framebuffer; enabling %s",
                                    (mode == SHIZViewportModeLetterbox ? "letterboxing" : "pillarboxing"));
        }
    }
}

static void _shiz_gfx_determine_viewport_mode(SHIZViewportMode *mode) {
    float const screen_aspect_ratio = _viewport.screen.width / _viewport.screen.height;
    float const framebuffer_aspect_ratio = _viewport.framebuffer.width / _viewport.framebuffer.height;

    *mode = SHIZViewportModeNormal;

    if (screen_aspect_ratio > framebuffer_aspect_ratio ||
        framebuffer_aspect_ratio > screen_aspect_ratio) {
        float const targetAspectRatio = screen_aspect_ratio;

        // letterbox (horizontal bars)
        float adjusted_width = _viewport.framebuffer.width;
        float adjusted_height = roundf(adjusted_width / targetAspectRatio);

        *mode = SHIZViewportModeLetterbox;

        if (adjusted_height > _viewport.framebuffer.height) {
            // pillarbox (vertical bars)
            adjusted_height = _viewport.framebuffer.height ;
            adjusted_width = roundf(adjusted_height * targetAspectRatio);

            *mode = SHIZViewportModePillarbox;
        }

        float const offset_width = _viewport.framebuffer.width - adjusted_width;
        float const offset_height = _viewport.framebuffer.height - adjusted_height;

        _viewport.aspect_ratio_offset = SHIZSizeMake(offset_width, offset_height);
    }
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

        shiz_io_error_context("GLSL", "compile error: %s", (char *)log);

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

        shiz_io_error_context("GLSL", "link error: %s", (char *)log);
        
        return 0;
    }
    
    return program;
}

#ifdef DEBUG
static uint _shiz_gfx_debug_draw_count = 0;

static void _shiz_gfx_debug_reset_draw_count() {
    _shiz_gfx_debug_draw_count = 0;
}

uint shiz_gfx_debug_get_draw_count() {
    return _shiz_gfx_debug_draw_count;
}

static void _shiz_gfx_debug_increment_draw_count(uint amount) {
    _shiz_gfx_debug_draw_count += amount;
}
#endif
