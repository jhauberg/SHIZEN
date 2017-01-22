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
#include <limits.h>

#include <linmath.h>

#include "gfx.h"
#include "io.h"

#ifdef SHIZ_DEBUG
static void _shiz_gfx_debug_reset_draw_count(void);
static void _shiz_gfx_debug_increment_draw_count(uint amount);
static void _shiz_gfx_debug_update_frame_stats(void);
static void _shiz_gfx_debug_update_frame_averages(void);
static SHIZVector3 _shiz_gfx_debug_get_last_sprite_origin(void);
static shiz_gfx_debug_event_callback * _shiz_gfx_debug_event;

static const char * const _shiz_gfx_debug_event_flush = "flush";
static const char * const _shiz_gfx_debug_event_flush_capacity = "flush (cap)";
static const char * const _shiz_gfx_debug_event_flush_texture_switch = "flush (tex)";

#endif

static GLuint _shiz_gfx_compile_shader(GLenum const type, const GLchar *source);
static GLuint _shiz_gfx_link_program(GLuint const vert, GLuint const frag);

static void _shiz_gfx_apply_viewport_boxing_if_necessary(void);
static void _shiz_gfx_determine_viewport_mode(SHIZViewportMode *mode);
static void _shiz_gfx_determine_operating_resolution(void);

static bool _shiz_gfx_init_primitive(void);
static bool _shiz_gfx_kill_primitive(void);

static void _shiz_gfx_primitive_state(bool const enable);

static bool _shiz_gfx_init_spritebatch(void);
static bool _shiz_gfx_kill_spritebatch(void);

static void _shiz_gfx_spritebatch_state(bool const enable);

static bool _shiz_gfx_spritebatch_flush(void);

static uint const spritebatch_max_count = 128; /* flush when reaching this limit */
static uint const spritebatch_vertex_count_per_quad = 2 * 3; /* 2 triangles per batched quad = 6 vertices  */
static uint const spritebatch_vertex_count = spritebatch_max_count * spritebatch_vertex_count_per_quad;

typedef struct SHIZRenderSpriteBatch {
    uint current_count;
    GLuint current_texture_id;
    SHIZRenderData render;
    SHIZVertexPositionColorTexture vertices[spritebatch_vertex_count];
} SHIZRenderSpriteBatch;

typedef struct SHIZRenderPrimitive {
    SHIZRenderData render;
} SHIZRenderPrimitive;

static SHIZRenderSpriteBatch _spritebatch;
static SHIZRenderPrimitive _primitive;

static SHIZViewport _viewport;

// set to false to let viewport fit framebuffer (pixels will be stretched)
static bool const enable_boxing_if_necessary = true;

bool shiz_gfx_init(SHIZViewport const viewport) {
    shiz_gfx_set_viewport(viewport);

    if (!_shiz_gfx_init_primitive()) {
        return false;
    }

    if (!_shiz_gfx_init_spritebatch()) {
        return false;
    }

    return true;
}

bool shiz_gfx_kill() {
    if (!_shiz_gfx_kill_primitive()) {
        return false;
    }

    if (!_shiz_gfx_kill_spritebatch()) {
        return false;
    }

    return true;
}

static bool _shiz_gfx_init_primitive() {
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
    
    _primitive.render.program = _shiz_gfx_link_program(vs, fs);
    
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

static bool _shiz_gfx_kill_primitive() {
    glDeleteProgram(_primitive.render.program);
    glDeleteVertexArrays(1, &_primitive.render.vao);
    glDeleteBuffers(1, &_primitive.render.vbo);
    
    return true;
}

static bool _shiz_gfx_init_spritebatch() {
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
    "    float threshold = 0.0001;\n"
    "    float s = texture_coord.s;\n"
    "    if (texture_coord_min.x > 0 && texture_coord_max.x > 0) {\n"
    "        s = mod(mod(s, texture_coord_min.x + threshold), "
    "                       texture_coord_max.x - threshold) + texture_coord_min.x;\n"
    "    } else if (texture_coord_max.x > 0) {\n"
    "        s = mod(s, texture_coord_max.x - threshold) + texture_coord_min.x;\n"
    "    }\n"
    "    float t = texture_coord.t;\n"
    "    if (texture_coord_min.y > 0 && texture_coord_max.y > 0) {\n"
    "        t = mod(mod(t, texture_coord_min.y + threshold),"
    "                       texture_coord_max.y - threshold) + texture_coord_min.y;\n"
    "    } else if (texture_coord_max.y > 0) {\n"
    "        t = mod(t, texture_coord_max.y - threshold) + texture_coord_min.y;\n"
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

    _spritebatch.render.program = _shiz_gfx_link_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!_spritebatch.render.program) {
        return false;
    }
    
    glGenBuffers(1, &_spritebatch.render.vbo);
    glGenVertexArrays(1, &_spritebatch.render.vao);

    glBindVertexArray(_spritebatch.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _spritebatch.render.vbo); {
            int const stride = sizeof(SHIZVertexPositionColorTexture);

            glBufferData(GL_ARRAY_BUFFER,
                         spritebatch_vertex_count * stride,
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
                                  // offset to reach color component
                                  (GLvoid*)(sizeof(SHIZVector3)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2 /* texture coord location */,
                                  2 /* number of coord components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  // offset to reach texture coord component
                                  (GLvoid*)(sizeof(SHIZVector3) +
                                            sizeof(SHIZColor)));
            glEnableVertexAttribArray(2);

            glVertexAttribPointer(3 /* texture coord scale location */,
                                  2 /* number of scale components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  // offset to reach texture coord min component
                                  (GLvoid*)(sizeof(SHIZVector3) +
                                            sizeof(SHIZColor) +
                                            sizeof(SHIZVector2)));
            glEnableVertexAttribArray(3);

            glVertexAttribPointer(4 /* texture coord scale location */,
                                  2 /* number of scale components per vertex */,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  stride,
                                  // offset to reach texture coord max component
                                  (GLvoid*)(sizeof(SHIZVector3) +
                                            sizeof(SHIZColor) +
                                            sizeof(SHIZVector2) +
                                            sizeof(SHIZVector2)));
            glEnableVertexAttribArray(4);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    return true;
}

static bool _shiz_gfx_kill_spritebatch() {
    glDeleteProgram(_spritebatch.render.program);
    glDeleteVertexArrays(1, &_spritebatch.render.vao);
    glDeleteBuffers(1, &_spritebatch.render.vbo);

    return true;
}

void shiz_gfx_clear() {
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shiz_gfx_begin() {
#ifdef SHIZ_DEBUG
    _shiz_gfx_debug_reset_draw_count();
#endif

    _spritebatch.current_texture_id = 0;
    
    glViewport(_viewport.offset.width / 2,
               _viewport.offset.height / 2,
               _viewport.framebuffer.width - _viewport.offset.width,
               _viewport.framebuffer.height - _viewport.offset.height);
}

void shiz_gfx_end() {
    shiz_gfx_flush();

#ifdef SHIZ_DEBUG
    _shiz_gfx_debug_update_frame_stats();
#endif
}

void shiz_gfx_flush() {
    if (_spritebatch.current_count > 0) {
#ifdef SHIZ_DEBUG
        SHIZVector3 const event_origin = _shiz_gfx_debug_get_last_sprite_origin();
#endif

        _shiz_gfx_spritebatch_flush();

#ifdef SHIZ_DEBUG
        if (_shiz_gfx_debug_event) {
            SHIZDebugEvent event;

            event.name = _shiz_gfx_debug_event_flush;
            event.origin = event_origin;
            event.lane = SHIZDebugEventLaneDraws;

            _shiz_gfx_debug_event(event);
        }
#endif
    }
}

static void mat4x4_model_view_projection(mat4x4 mvp, mat4x4 model, mat4x4 view) {
    mat4x4 projection;
    mat4x4_ortho(projection,
                 0, _viewport.screen.width,
                 0, _viewport.screen.height,
                 -1 /* near */, 1 /* far */);

    mat4x4 view_model;
    mat4x4_mul(view_model, view, model);

    mat4x4_mul(mvp, projection, view_model);
}

static void _shiz_gfx_primitive_state(bool const enable) {
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

void shiz_gfx_render(GLenum const mode,
                     SHIZVertexPositionColor const * restrict vertices,
                     uint const count) {
    shiz_gfx_render_ex(mode, vertices, count, SHIZVector3Zero, 0);
}

void shiz_gfx_render_ex(GLenum const mode,
                        SHIZVertexPositionColor const * restrict vertices,
                        uint const count,
                        SHIZVector3 const origin,
                        float const angle) {
    mat4x4 translation;
    mat4x4_translate(translation, origin.x, origin.y, origin.z);
    
    mat4x4 rotation;
    mat4x4_identity(rotation);
    mat4x4_rotate_Z(rotation, rotation, angle);
    
    mat4x4 model;
    mat4x4_mul(model, translation, rotation);
    
    mat4x4 view;
    mat4x4_identity(view);
    
    mat4x4 mvp;
    mat4x4_model_view_projection(mvp, model, view);
    
    _shiz_gfx_primitive_state(true);
    
    glUseProgram(_primitive.render.program);
    glUniformMatrix4fv(glGetUniformLocation(_primitive.render.program, "transform_mvp"), 1, GL_FALSE, *mvp);
    glBindVertexArray(_primitive.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _primitive.render.vbo); {
            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(SHIZVertexPositionColor) * count,
                         vertices,
                         GL_DYNAMIC_DRAW);
            glDrawArrays(mode, 0, count /* count of indices; not count of lines; i.e. 1 line = 2 vertices/indices */);
#ifdef SHIZ_DEBUG
            _shiz_gfx_debug_increment_draw_count(1);
#endif
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);

    _shiz_gfx_primitive_state(false);
}

void shiz_gfx_render_quad(SHIZVertexPositionColorTexture const * restrict vertices,
                          SHIZVector3 const origin,
                          float const angle,
                          GLuint const texture_id) {
    if (_spritebatch.current_texture_id != 0 && /* dont flush if texture is not set yet */
        _spritebatch.current_texture_id != texture_id) {
        bool const flushed = _shiz_gfx_spritebatch_flush();

#ifdef SHIZ_DEBUG
        if (flushed && _shiz_gfx_debug_event) {
            SHIZDebugEvent event;

            event.name = _shiz_gfx_debug_event_flush_texture_switch;
            event.origin = origin;
            event.lane = SHIZDebugEventLaneDraws;

            _shiz_gfx_debug_event(event);
        }
#endif
    }

    _spritebatch.current_texture_id = texture_id;

    if (_spritebatch.current_count + 1 > spritebatch_max_count) {
        bool const flushed = _shiz_gfx_spritebatch_flush();

#ifdef SHIZ_DEBUG
        if (flushed && _shiz_gfx_debug_event) {
            SHIZDebugEvent event;

            event.name = _shiz_gfx_debug_event_flush_capacity;
            event.origin = origin;
            event.lane = SHIZDebugEventLaneDraws;

            _shiz_gfx_debug_event(event);
        }
#endif
    }

    uint const offset = _spritebatch.current_count * spritebatch_vertex_count_per_quad;

    mat4x4 translation;
    mat4x4_translate(translation, origin.x, origin.y, origin.z);

    mat4x4 rotation;
    mat4x4_identity(rotation);
    mat4x4_rotate_Z(rotation, rotation, angle);

    mat4x4 world;
    mat4x4_mul(world, translation, rotation);

    for (uint i = 0; i < spritebatch_vertex_count_per_quad; i++) {
        SHIZVertexPositionColorTexture vertex = vertices[i];

        vec4 position = {
            vertex.position.x,
            vertex.position.y,
            vertex.position.z, 1
        };

        vec4 transformed_position;
        mat4x4_mul_vec4(transformed_position, world, position);
        
        vertex.position = SHIZVector3Make(transformed_position[0],
                                          transformed_position[1],
                                          transformed_position[2]);
        
        _spritebatch.vertices[offset + i] = vertex;
    }

    _spritebatch.current_count += 1;
}

static void _shiz_gfx_spritebatch_state(bool const enable) {
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
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }
}

static bool _shiz_gfx_spritebatch_flush() {
    if (_spritebatch.current_count == 0) {
        return false;
    }

    mat4x4 model;
    mat4x4_identity(model);
    mat4x4 view;
    mat4x4_identity(view);
    
    mat4x4 mvp;
    mat4x4_model_view_projection(mvp, model, view);

    _shiz_gfx_spritebatch_state(true);
    
    glUseProgram(_spritebatch.render.program);
    // todo: a way to provide this flag; problem is that it affects the entire batch
    glUniform1i(glGetUniformLocation(_spritebatch.render.program, "enable_additive_tint"), false);
    glUniformMatrix4fv(glGetUniformLocation(_spritebatch.render.program, "transform_mvp"), 1, GL_FALSE, *mvp);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _spritebatch.current_texture_id); {
        glBindVertexArray(_spritebatch.render.vao); {
            glBindBuffer(GL_ARRAY_BUFFER, _spritebatch.render.vbo); {
                uint const index_count = _spritebatch.current_count * spritebatch_vertex_count_per_quad;

                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                index_count * sizeof(SHIZVertexPositionColorTexture),
                                _spritebatch.vertices);
                glDrawArrays(GL_TRIANGLES, 0, index_count);
#ifdef SHIZ_DEBUG
                _shiz_gfx_debug_increment_draw_count(1);
#endif
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    _shiz_gfx_spritebatch_state(false);

    _spritebatch.current_count = 0;

    return true;
}

SHIZViewport shiz_gfx_get_viewport() {
    return _viewport;
}

void shiz_gfx_set_viewport(SHIZViewport const viewport) {
    if (viewport.framebuffer.width == _viewport.framebuffer.width ||
        viewport.framebuffer.height == _viewport.framebuffer.height) {
        return;
    }

    _viewport = viewport;

    _shiz_gfx_determine_operating_resolution();
    _shiz_gfx_apply_viewport_boxing_if_necessary();
}

static void _shiz_gfx_determine_operating_resolution() {
    if ((_viewport.screen.width < _viewport.framebuffer.width ||
         _viewport.screen.width > _viewport.framebuffer.width) ||
        (_viewport.screen.height < _viewport.framebuffer.height ||
         _viewport.screen.height > _viewport.framebuffer.height)) {
        shiz_io_warning_context("GFX", "Operating resolution is %.0fx%.0f @ %.0fx%.0f@%.0fx (%s)",
                                _viewport.screen.width, _viewport.screen.height,
                                _viewport.framebuffer.width, _viewport.framebuffer.height,
                                _viewport.scale,
                                _viewport.is_fullscreen ? "fullscreen" : "windowed");
    }
}

static void _shiz_gfx_apply_viewport_boxing_if_necessary() {
    _viewport.offset = SHIZSizeEmpty;

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

        _viewport.offset = SHIZSizeMake(_viewport.framebuffer.width - adjusted_width,
                                        _viewport.framebuffer.height - adjusted_height);
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

#ifdef SHIZ_DEBUG
static SHIZVector3 _shiz_gfx_debug_get_last_sprite_origin() {
    if (_spritebatch.current_count > 0) {
        int const offset = (_spritebatch.current_count - 1) * spritebatch_vertex_count_per_quad;

        if (offset >= 0) {
            SHIZVector3 last_sprite_bl_vertex = _spritebatch.vertices[offset + 2].position;
            SHIZVector3 last_sprite_tr_vertex = _spritebatch.vertices[offset + 4].position;

            SHIZVector3 const mid_point = SHIZVector3Make((last_sprite_bl_vertex.x + last_sprite_tr_vertex.x) / 2,
                                                          (last_sprite_bl_vertex.y + last_sprite_tr_vertex.y) / 2,
                                                          (last_sprite_bl_vertex.z + last_sprite_tr_vertex.z) / 2);

            return mid_point;
        }
    }

    return SHIZVector3Zero;
}

void shiz_gfx_debug_set_event_callback(shiz_gfx_debug_event_callback * const callback) {
    _shiz_gfx_debug_event = callback;
}

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

static double const _shiz_gfx_debug_average_interval = 1.0; // in seconds
static double _shiz_gfx_debug_last_average_time = 0;

static uint _shiz_gfx_debug_frame_samples = 0; // sample frames to calculate average
static uint _shiz_gfx_debug_frame_sample_count = 0;

static uint _shiz_gfx_debug_frames_per_second = 0;
static uint _shiz_gfx_debug_frames_per_second_min = UINT_MAX;
static uint _shiz_gfx_debug_frames_per_second_max = 0;
static uint _shiz_gfx_debug_frames_per_second_avg = 0;

static double _shiz_gfx_debug_frame_time = 0;
static double _shiz_gfx_debug_frame_time_avg = 0;

static double _shiz_gfx_debug_frame_time_samples = 0; // sample frames to calculate average
static uint _shiz_gfx_debug_frame_time_sample_count = 0;

static double _shiz_gfx_debug_last_frame_time = 0;

double shiz_gfx_debug_get_frame_time() {
    return _shiz_gfx_debug_frame_time * 1000;
}

double shiz_gfx_debug_get_frame_time_avg() {
    return _shiz_gfx_debug_frame_time_avg * 1000;
}

uint shiz_gfx_debug_get_frames_per_second() {
    return _shiz_gfx_debug_frames_per_second;
}

uint shiz_gfx_debug_get_frames_per_second_max() {
    return _shiz_gfx_debug_frames_per_second_max;
}

uint shiz_gfx_debug_get_frames_per_second_min() {
    return _shiz_gfx_debug_frames_per_second_min;
}

uint shiz_gfx_debug_get_frames_per_second_avg() {
    return _shiz_gfx_debug_frames_per_second_avg;
}

static void _shiz_gfx_debug_update_frame_averages() {
    _shiz_gfx_debug_frames_per_second_avg = _shiz_gfx_debug_frame_samples / _shiz_gfx_debug_frame_sample_count;
    _shiz_gfx_debug_frame_sample_count = 0;
    _shiz_gfx_debug_frame_samples = 0;
    
    _shiz_gfx_debug_frame_time_avg = _shiz_gfx_debug_frame_time_samples / _shiz_gfx_debug_frame_time_sample_count;
    _shiz_gfx_debug_frame_time_sample_count = 0;
    _shiz_gfx_debug_frame_time_samples = 0;
    
    // reset min/max to show rolling stats rather than historically accurate stats (its more interesting
    // knowing min/max for the current scene/context than knowing the 9999+ max fps during the first blank frame)
    _shiz_gfx_debug_frames_per_second_min = UINT_MAX;
    _shiz_gfx_debug_frames_per_second_max = 0;
}

static void _shiz_gfx_debug_update_frame_stats() {
    double const time = glfwGetTime();
    double const time_since_last_frame = time - _shiz_gfx_debug_last_frame_time;

    _shiz_gfx_debug_frame_time = time_since_last_frame;
    _shiz_gfx_debug_last_frame_time = time;
    
    _shiz_gfx_debug_frames_per_second = 1.0 / _shiz_gfx_debug_frame_time;
    
    if (_shiz_gfx_debug_frames_per_second < _shiz_gfx_debug_frames_per_second_min) {
        _shiz_gfx_debug_frames_per_second_min = _shiz_gfx_debug_frames_per_second;
    }

    if (_shiz_gfx_debug_frames_per_second > _shiz_gfx_debug_frames_per_second_max) {
        _shiz_gfx_debug_frames_per_second_max = _shiz_gfx_debug_frames_per_second;
    }

    _shiz_gfx_debug_frame_samples += _shiz_gfx_debug_frames_per_second;
    _shiz_gfx_debug_frame_sample_count++;

    _shiz_gfx_debug_frame_time_samples += _shiz_gfx_debug_frame_time;
    _shiz_gfx_debug_frame_time_sample_count++;
    
    double const time_since_last_average = time - _shiz_gfx_debug_last_average_time;

    if (time_since_last_average >= _shiz_gfx_debug_average_interval) {
        _shiz_gfx_debug_last_average_time = time;

        _shiz_gfx_debug_update_frame_averages();
    }
}

#endif
