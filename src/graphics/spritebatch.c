#include "spritebatch.h"

#include "shader.h"
#include "viewport.h"
#include "transform.h"

#ifdef SHIZ_DEBUG
 #include "../debug/profiler.h" // z_profiler_*
#endif

#define SPRITES_MAX 128 /* flush when reaching this limit */

#define VERTEX_COUNT_PER_SPRITE (2 * 3) /* 2 triangles per batched quad = 6 vertices */

#define VERTEX_COUNT_PER_BATCH (SPRITES_MAX * VERTEX_COUNT_PER_SPRITE)

static void z_gfx__spritebatch_state(bool enable);

typedef struct SHIZSpriteBatch {
    SHIZVertexPositionColorTexture vertices[VERTEX_COUNT_PER_BATCH];
    SHIZRenderObject render;
    GLuint texture_id;
    uint16_t count;
} SHIZSpriteBatch;

static SHIZSpriteBatch _spritebatch;

bool
z_gfx__init_spritebatch()
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

    GLuint const vs = z_gfx__compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint const fs = z_gfx__compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
    
    if (!vs && !fs) {
        return false;
    }
    
    _spritebatch.render.program = z_gfx__link_program(vs, fs);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    if (!_spritebatch.render.program) {
        return false;
    }
    
    glGenBuffers(1, &_spritebatch.render.vbo);
    glGenVertexArrays(1, &_spritebatch.render.vao);
    
    glBindVertexArray(_spritebatch.render.vao); {
        glBindBuffer(GL_ARRAY_BUFFER, _spritebatch.render.vbo); {
            GLsizei const stride = sizeof(SHIZVertexPositionColorTexture);
            
            glBufferData(GL_ARRAY_BUFFER,
                         VERTEX_COUNT_PER_BATCH * stride,
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
z_gfx__kill_spritebatch()
{
    glDeleteProgram(_spritebatch.render.program);
    glDeleteVertexArrays(1, &_spritebatch.render.vao);
    glDeleteBuffers(1, &_spritebatch.render.vbo);
    
    return true;
}

void
z_gfx__add_sprite(SHIZVertexPositionColorTexture const * restrict const vertices,
                  SHIZVector3 const origin,
                  float const angle,
                  GLuint const texture_id)
{
    if (_spritebatch.texture_id != 0 && /* dont flush if texture is not set yet */
        _spritebatch.texture_id != texture_id) {
        if (z_gfx__spritebatch_flush()) {
            // texture switch requires flushing
        }
    }
    
    _spritebatch.texture_id = texture_id;
    
    if (_spritebatch.count + 1 > SPRITES_MAX) {
        if (z_gfx__spritebatch_flush()) {
            // hitting capacity requires flushing
        }
    }
    
    uint32_t const offset = _spritebatch.count * VERTEX_COUNT_PER_SPRITE;
    
    mat4x4 transform;
    
    z_transform__translate_rotate_scale(transform, origin, angle, 1);
    
    for (uint8_t v = 0; v < VERTEX_COUNT_PER_SPRITE; v++) {
        SHIZVertexPositionColorTexture vertex = vertices[v];
        
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
        
        _spritebatch.vertices[offset + v] = vertex;
    }
    
    _spritebatch.count += 1;
}

bool
z_gfx__spritebatch_flush()
{
    if (_spritebatch.count == 0) {
        return false;
    }
    
    mat4x4 model;
    mat4x4_identity(model);
    
    mat4x4 transform;

    // todo: optimization; in many cases we don't have to keep building the projection matrix
    //                     because it only changes when the viewport changes- which is probably not every frame
    z_transform__project_ortho(transform, model, z_viewport__get());
    
    z_gfx__spritebatch_state(true);
    
    glUseProgram(_spritebatch.render.program);
    // todo: a way to provide this flag; problem is that it affects the entire batch
    glUniform1i(glGetUniformLocation(_spritebatch.render.program, "enable_additive_tint"), false);
    glUniformMatrix4fv(glGetUniformLocation(_spritebatch.render.program, "transform"), 1, GL_FALSE, *transform);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _spritebatch.texture_id); {
        glBindVertexArray(_spritebatch.render.vao); {
            glBindBuffer(GL_ARRAY_BUFFER, _spritebatch.render.vbo); {
                GLsizei const count = _spritebatch.count * VERTEX_COUNT_PER_SPRITE;
                GLsizeiptr const size = sizeof(SHIZVertexPositionColorTexture) * (uint32_t)count;
                
                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                size,
                                _spritebatch.vertices);
                glDrawArrays(GL_TRIANGLES, 0, count);
#ifdef SHIZ_DEBUG
                z_profiler__increment_draw_count(1);
#endif
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    z_gfx__spritebatch_state(false);
    
    _spritebatch.count = 0;
    
    return true;
}

void
z_gfx__spritebatch_reset()
{
    _spritebatch.count = 0;
    _spritebatch.texture_id = 0;
}

static
void
z_gfx__spritebatch_state(bool const enable)
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
