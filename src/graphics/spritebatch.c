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

#include "spritebatch.h"

#include "shader.h"
#include "viewport.h"
#include "transform.h"

#ifdef SHIZ_DEBUG
 #include "debug.h"
#endif

#ifdef SHIZ_DEBUG
static SHIZVector3 _shiz_debug_get_last_sprite_origin(void);

static const char * const _shiz_debug_event_flush = "fls";
static const char * const _shiz_debug_event_flush_capacity = "fls|cap";
static const char * const _shiz_debug_event_flush_texture_switch = "fls|tex";
#endif

static void _shiz_gfx_spritebatch_state(bool enable);

#define SHIZGFXSpriteBatchMax 128 /* flush when reaching this limit */

static unsigned int const spritebatch_vertex_count_per_quad = 2 * 3; /* 2 triangles per batched quad = 6 vertices  */
static unsigned int const spritebatch_vertex_count = SHIZGFXSpriteBatchMax * spritebatch_vertex_count_per_quad;

typedef struct SHIZSpriteBatch {
    SHIZVertexPositionColorTexture vertices[spritebatch_vertex_count];
    SHIZRenderObject render;
    GLuint texture_id;
    unsigned int count;
} SHIZSpriteBatch;

static SHIZSpriteBatch _spritebatch;

void
shiz_gfx_add_sprite(SHIZVertexPositionColorTexture const * restrict const vertices,
                    SHIZVector3 const origin,
                    float const angle,
                    GLuint const texture_id)
{
    if (_spritebatch.texture_id != 0 && /* dont flush if texture is not set yet */
        _spritebatch.texture_id != texture_id) {
        if (shiz_gfx_spritebatch_flush()) {
#ifdef SHIZ_DEBUG
            shiz_debug_add_event_draw(_shiz_debug_event_flush_texture_switch, origin);
#endif
        }
    }
    
    _spritebatch.texture_id = texture_id;
    
    if (_spritebatch.count + 1 > SHIZGFXSpriteBatchMax) {
        if (shiz_gfx_spritebatch_flush()) {
#ifdef SHIZ_DEBUG
            shiz_debug_add_event_draw(_shiz_debug_event_flush_capacity, origin);
#endif
        }
    }
    
    unsigned int const offset = _spritebatch.count * spritebatch_vertex_count_per_quad;
    
    mat4x4 transform;
    
    shiz_transform_translate_rotate_scale(transform, origin, angle, 1);
    
    for (unsigned int i = 0; i < spritebatch_vertex_count_per_quad; i++) {
        SHIZVertexPositionColorTexture vertex = vertices[i];
        
        vec4 position = {
            vertex.position.x,
            vertex.position.y,
            vertex.position.z, 1
        };
        
        vec4 world_position;
        
        mat4x4_mul_vec4(world_position, transform, position);
        
        vertex.position = SHIZVector3Make(world_position[0],
                                          world_position[1],
                                          world_position[2]);
        
        _spritebatch.vertices[offset + i] = vertex;
    }
    
    _spritebatch.count += 1;
}

bool
shiz_gfx_init_spritebatch()
{
    char const * const vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in vec4 vertex_color;\n"
    "layout (location = 2) in vec2 vertex_texture_coord;\n"
    "layout (location = 3) in vec2 vertex_texture_coord_min;\n"
    "layout (location = 4) in vec2 vertex_texture_coord_max;\n"
    "uniform mat4 transform;\n"
    "out vec2 texture_coord;\n"
    "out vec2 texture_coord_min;\n"
    "out vec2 texture_coord_max;\n"
    "out vec4 tint_color;\n"
    "void main() {\n"
    "    gl_Position = transform * vec4(vertex_position, 1);\n"
    "    texture_coord = vertex_texture_coord.st;\n"
    "    texture_coord_min = vertex_texture_coord_min.st;\n"
    "    texture_coord_max = vertex_texture_coord_max.st;\n"
    "    tint_color = vertex_color;\n"
    "}\n";

    char const * const fragment_shader =
    "#version 330 core\n"
    "in vec2 texture_coord;\n"
    "in vec2 texture_coord_min;\n"
    "in vec2 texture_coord_max;\n"
    "in vec4 tint_color;\n"
    "uniform int enable_additive_tint;\n"
    "uniform sampler2D sampler;\n"
    "layout (location = 0) out vec4 fragment_color;\n"
    "void main() {\n"
    "    vec2 rollover_texture_coord = mod(texture_coord_min - texture_coord,\n"
    "                                      texture_coord_max - texture_coord_min);\n"
    "    vec2 repeated_texture_coord = texture_coord_max - rollover_texture_coord;\n"
    "    vec4 sampled_color = texture(sampler, repeated_texture_coord.st);\n"
    "    if (enable_additive_tint != 0) {\n"
    "        fragment_color = (sampled_color + vec4(tint_color.rgb, 0)) * tint_color.a;\n"
    "    } else {\n"
    "        fragment_color = sampled_color * tint_color;\n"
    "    }\n"
    "}";

    GLuint const vs = shiz_gfx_compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint const fs = shiz_gfx_compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
    
    if (!vs && !fs) {
        return false;
    }
    
    _spritebatch.render.program = shiz_gfx_link_program(vs, fs);
    
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
                                  GL_FLOAT, GL_FALSE /* values are not normalized */,
                                  stride /* offset to reach next vertex */,
                                  0 /* position component is the first, so no offset */);
            glEnableVertexAttribArray(0);
            
            glVertexAttribPointer(1 /* color location */,
                                  4 /* number of color components per vertex */,
                                  GL_FLOAT, GL_FALSE,
                                  stride,
                                  // offset to reach color component
                                  (GLvoid*)(sizeof(SHIZVector3)));
            glEnableVertexAttribArray(1);
            
            glVertexAttribPointer(2 /* texture coord location */,
                                  2 /* number of coord components per vertex */,
                                  GL_FLOAT, GL_FALSE,
                                  stride,
                                  // offset to reach texture coord component
                                  (GLvoid*)(sizeof(SHIZVector3) +
                                            sizeof(SHIZColor)));
            glEnableVertexAttribArray(2);
            
            glVertexAttribPointer(3 /* texture coord scale location */,
                                  2 /* number of scale components per vertex */,
                                  GL_FLOAT, GL_FALSE,
                                  stride,
                                  // offset to reach texture coord min component
                                  (GLvoid*)(sizeof(SHIZVector3) +
                                            sizeof(SHIZColor) +
                                            sizeof(SHIZVector2)));
            glEnableVertexAttribArray(3);
            
            glVertexAttribPointer(4 /* texture coord scale location */,
                                  2 /* number of scale components per vertex */,
                                  GL_FLOAT, GL_FALSE,
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

bool
shiz_gfx_kill_spritebatch()
{
    glDeleteProgram(_spritebatch.render.program);
    glDeleteVertexArrays(1, &_spritebatch.render.vao);
    glDeleteBuffers(1, &_spritebatch.render.vbo);
    
    return true;
}

bool
shiz_gfx_spritebatch_flush()
{
    if (_spritebatch.count == 0) {
        return false;
    }
    
#ifdef SHIZ_DEBUG
    // note that this will use the center of the sprite, which may not correlate to the anchor
    // that this sprite was drawn with
    shiz_debug_add_event_draw(_shiz_debug_event_flush,
                              _shiz_debug_get_last_sprite_origin());
#endif
    
    mat4x4 model;
    mat4x4_identity(model);
    
    mat4x4 transform;

    shiz_transform_project_ortho(transform, model, shiz_get_viewport());
    
    _shiz_gfx_spritebatch_state(true);
    
    glUseProgram(_spritebatch.render.program);
    // todo: a way to provide this flag; problem is that it affects the entire batch
    glUniform1i(glGetUniformLocation(_spritebatch.render.program, "enable_additive_tint"), false);
    glUniformMatrix4fv(glGetUniformLocation(_spritebatch.render.program, "transform"), 1, GL_FALSE, *transform);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _spritebatch.texture_id); {
        glBindVertexArray(_spritebatch.render.vao); {
            glBindBuffer(GL_ARRAY_BUFFER, _spritebatch.render.vbo); {
                unsigned int const index_count =
                    _spritebatch.count * spritebatch_vertex_count_per_quad;
                
                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                index_count * sizeof(SHIZVertexPositionColorTexture),
                                _spritebatch.vertices);
                glDrawArrays(GL_TRIANGLES, 0, index_count);
#ifdef SHIZ_DEBUG
                shiz_debug_increment_draw_count(1);
#endif
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    _shiz_gfx_spritebatch_state(false);
    
    _spritebatch.count = 0;
    
    return true;
}

void
shiz_gfx_spritebatch_reset()
{
    _spritebatch.count = 0;
    _spritebatch.texture_id = 0;
}

static
void
_shiz_gfx_spritebatch_state(bool const enable)
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
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }
}

#ifdef SHIZ_DEBUG
static
SHIZVector3
_shiz_debug_get_last_sprite_origin()
{
    if (_spritebatch.count > 0) {
        int const offset = (_spritebatch.count - 1) * spritebatch_vertex_count_per_quad;
        
        if (offset >= 0) {
            SHIZVector3 const last_sprite_bl_vertex = _spritebatch.vertices[offset + 2].position;
            SHIZVector3 const last_sprite_tr_vertex = _spritebatch.vertices[offset + 4].position;
            
            SHIZVector3 const mid_point = SHIZVector3Make((last_sprite_bl_vertex.x + last_sprite_tr_vertex.x) / 2,
                                                          (last_sprite_bl_vertex.y + last_sprite_tr_vertex.y) / 2,
                                                          (last_sprite_bl_vertex.z + last_sprite_tr_vertex.z) / 2);
            
            return mid_point;
        }
    }
    
    return SHIZVector3Zero;
}
#endif
