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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SHIZEN/engine.h>

#include "internal.h"
#include "gfx.h"
#include "res.h"
#include "io.h"

#define SHIZ_MIN_OPENGL_VERSION_MAJOR 3
#define SHIZ_MIN_OPENGL_VERSION_MINOR 3

static void _shiz_glfw_error_callback(int error, const char* description);

static void _shiz_glfw_window_close_callback(GLFWwindow* window);
static void _shiz_glfw_window_focus_callback(GLFWwindow* window, int focused);

static void _shiz_glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);

static SHIZSize _shiz_glfw_get_window_size(void);
static SHIZSize _shiz_glfw_get_framebuffer_size(void);

static SHIZViewport _shiz_get_viewport(void);

static float _shiz_glfw_get_pixel_scale(void);

static void _shiz_intro(void);
static bool _shiz_can_run(void);

static void _shiz_draw_rect(SHIZRect const rect, SHIZColor const color, bool const fill);

static SHIZSpriteFontMeasurement _shiz_measure_sprite_text(SHIZSpriteFont const font, const char* text, SHIZSize const bounds, SHIZVector2 const scale, float const spread);

#ifdef SHIZ_DEBUG
static void _shiz_debug_process_errors(void);
#endif

static SHIZGraphicsContext context;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        context.should_finish = true;
    }
}

bool shiz_startup(SHIZWindowSettings const settings) {
    if (context.is_initialized) {
        return true;
    }

    context.preferred_screen_size = settings.size;
    
    glfwSetErrorCallback(_shiz_glfw_error_callback);
    
    if (!glfwInit()) {
        shiz_io_error_context("GLFW", "(%s) failed to initialize", glfwGetVersionString());

        return false;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SHIZ_MIN_OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SHIZ_MIN_OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if (settings.fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        context.window = glfwCreateWindow(mode->width, mode->height,
                                          settings.title, glfwGetPrimaryMonitor(), NULL);
    } else {
        context.window = glfwCreateWindow(settings.size.width, settings.size.height,
                                          settings.title, NULL, NULL);
    }

    if (!context.window) {
        shiz_io_error_context("GLFW", "(%s) failed to create window", glfwGetVersionString());

        return false;
    }

    glfwSetWindowCloseCallback(context.window, _shiz_glfw_window_close_callback);
    glfwSetWindowFocusCallback(context.window, _shiz_glfw_window_focus_callback);

    glfwSetFramebufferSizeCallback(context.window, _shiz_glfw_framebuffer_size_callback);

    glfwSetKeyCallback(context.window, key_callback);

    glfwMakeContextCurrent(context.window);
    glfwSwapInterval(settings.vsync ? 1 : 0);

    if (gl3wInit()) {
        shiz_io_error_context("gl3w", "failed to initialize");
        
        return false;
    }
    
    if (!_shiz_can_run()) {
        shiz_io_error("SHIZEN is not supported on this system");

        return false;
    }

    _shiz_intro();
    
    if (!shiz_gfx_init(_shiz_get_viewport())) {
        return false;
    }
    
    context.is_initialized = true;

    return true;
}

bool shiz_shutdown() {
    if (!context.is_initialized) {
        return false;
    }

    if (!shiz_gfx_kill()) {
        return false;
    }

    shiz_res_unload_all();
    
    glfwTerminate();
    
    context.is_initialized = false;
    
    return true;
}

void shiz_request_finish() {
    context.should_finish = true;
}

bool shiz_should_finish() {
    return context.should_finish;
}

uint shiz_load(const char *filename) {
    return shiz_res_load(shiz_res_get_type(filename), filename);
}

bool shiz_unload(uint const resource_id) {
    return shiz_res_unload(resource_id);
}

SHIZSprite shiz_load_sprite(uint const resource_id) {
    SHIZResourceImage const image = shiz_res_get_image(resource_id);

    SHIZRect const source = SHIZRectMake(SHIZVector2Zero,
                                         SHIZSizeMake(image.width, image.height));

    return shiz_load_sprite_src(resource_id, source);
}

SHIZSprite shiz_load_sprite_src(uint const resource_id, SHIZRect source) {
    SHIZSprite sprite;

    sprite.resource_id = resource_id;
    sprite.source = source;

    return sprite;
}

SHIZSpriteFont shiz_load_sprite_font(SHIZSprite const sprite, SHIZSize const character, SHIZASCIITable const table) {
    SHIZSpriteFont spritefont;
    
    spritefont.sprite = sprite;
    spritefont.character = character;
    spritefont.table = table;
    
    return spritefont;
}

void shiz_drawing_begin() {
    shiz_gfx_clear();
    shiz_gfx_begin();
}

void shiz_drawing_end() {
    shiz_gfx_end();

#ifdef SHIZ_DEBUG
    //printf("draw calls this frame: %d\n", shiz_gfx_debug_get_draw_count());
    _shiz_debug_process_errors();
#endif

    glfwSwapBuffers(context.window);
    glfwPollEvents();
}

static void shiz_draw_path_3d(SHIZVector3 const points[], uint const count, SHIZColor const color) {
    SHIZVertexPositionColor vertices[count];

    for (uint i = 0; i < count; i++) {
        vertices[i].position = points[i];
        vertices[i].color = color;
    }

    shiz_gfx_render(GL_LINE_STRIP, vertices, count);
}

static void shiz_draw_line_3d(SHIZVector3 const from, SHIZVector3 const to, SHIZColor const color) {
    SHIZVector3 points[] = {
        from, to
    };

    shiz_draw_path_3d(points, 2, color);
}

void shiz_draw_line(SHIZVector2 const from, SHIZVector2 const to, SHIZColor const color) {
    shiz_draw_line_3d(SHIZVector3Make(from.x, from.y, 0),
                   SHIZVector3Make(to.x, to.y, 0),
                   color);
}

void shiz_draw_path(SHIZVector2 const points[], uint const count, SHIZColor const color) {
    SHIZVector3 points3[count];

    for (uint i = 0; i < count; i++) {
        points3[i].x = points[i].x;
        points3[i].y = points[i].y;
    }

    shiz_draw_path_3d(points3, count, color);
}

static void _shiz_draw_rect(SHIZRect const rect, SHIZColor const color, bool const fill) {
    uint const vertex_count = fill ? 4 : 5; // only drawing the shape requires an additional vertex
    
    SHIZVertexPositionColor vertices[vertex_count];
    
    for (uint i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
    }
    
    float const l = rect.origin.x;
    float const r = rect.origin.x + rect.size.width;
    float const b = rect.origin.y;
    float const t = rect.origin.y + rect.size.height;
    
    if (!fill) {
        vertices[0].position = SHIZVector3Make(l, b, 0);
        vertices[1].position = SHIZVector3Make(l, t, 0);
        // note that the order of the vertices differ from the filled shape
        vertices[2].position = SHIZVector3Make(r, t, 0);
        vertices[3].position = SHIZVector3Make(r, b, 0);
        // the additional vertex connects to the beginning, to complete the shape
        vertices[4].position = vertices[0].position;
        
        shiz_gfx_render(GL_LINE_STRIP, vertices, vertex_count);
    } else {
        vertices[0].position = SHIZVector3Make(l, b, 0);
        vertices[1].position = SHIZVector3Make(l, t, 0);
        vertices[2].position = SHIZVector3Make(r, b, 0);
        vertices[3].position = SHIZVector3Make(r, t, 0);
        
        shiz_gfx_render(GL_TRIANGLE_STRIP, vertices, vertex_count);
    }
}

void shiz_draw_rect(SHIZRect const rect, SHIZColor const color) {
    _shiz_draw_rect(rect, color, true);
}

void shiz_draw_rect_shape(SHIZRect const rect, SHIZColor const color) {
    _shiz_draw_rect(rect, color, false);
}

void shiz_draw_sprite(SHIZSprite const sprite, SHIZVector2 const origin) {
    shiz_draw_sprite_ex(sprite, origin,
                        SHIZSpriteSizeIntrinsic,
                        SHIZSpriteAnchorCenter,
                        SHIZSpriteNoAngle,
                        SHIZSpriteNoTint,
                        SHIZSpriteNoRepeat);
}

void shiz_draw_sprite_ex(SHIZSprite const sprite, SHIZVector2 const origin, SHIZSize const size, SHIZVector2 const anchor, float const angle, SHIZColor const tint, bool const repeat) {
    SHIZResourceImage image = shiz_res_get_image(sprite.resource_id);

    if (image.id == sprite.resource_id) {
        uint const vertex_count = 6;

        SHIZVertexPositionColorTexture vertices[vertex_count];

        for (uint i = 0; i < vertex_count; i++) {
            vertices[i].color = tint;
        }

        SHIZSize const working_size = (size.width == SHIZSpriteSizeIntrinsic.width &&
                                       size.height == SHIZSpriteSizeIntrinsic.height) ?
                                        sprite.source.size : size;

        float const hw = working_size.width / 2;
        float const hh = working_size.height / 2;
        
        // the anchor point determines what the origin means;
        // i.e. the origin becomes the point of which the sprite is drawn and rotated
        float const dx = hw * -anchor.x;
        float const dy = hh * -anchor.y;

        SHIZVector2 bl = SHIZVector2Make(dx - hw, dy - hh);
        SHIZVector2 tl = SHIZVector2Make(dx - hw, dy + hh);
        SHIZVector2 tr = SHIZVector2Make(dx + hw, dy + hh);
        SHIZVector2 br = SHIZVector2Make(dx + hw, dy - hh);

        vertices[0].position = SHIZVector3Make(tl.x, tl.y, 0);
        vertices[1].position = SHIZVector3Make(br.x, br.y, 0);
        vertices[2].position = SHIZVector3Make(bl.x, bl.y, 0);

        vertices[3].position = SHIZVector3Make(tl.x, tl.y, 0);
        vertices[4].position = SHIZVector3Make(tr.x, tr.y, 0);
        vertices[5].position = SHIZVector3Make(br.x, br.y, 0);
        
        SHIZRect source = sprite.source;
        
        bool const flip_vertically = true;
        
        if (flip_vertically) {
            // opengl assumes that the origin of textures is at the bottom-left of the image,
            // however, it is common to specify top-left as origin when using e.g. sprite sheets (and we want that)
            // so, assuming that the provided source frame expects the top-left to be the origin,
            // we have to flip the specified coordinate so that the origin becomes bottom-left
            source.origin.y = (image.height - source.size.height) - source.origin.y;
        }
        
        SHIZVector2 const uv_min = SHIZVector2Make((source.origin.x / image.width),
                                                   (source.origin.y / image.height));
        SHIZVector2 const uv_max = SHIZVector2Make(((source.origin.x + source.size.width) / image.width),
                                                   ((source.origin.y + source.size.height) / image.height));

        float uv_scale_x = 1;
        float uv_scale_y = 1;
        
        if (repeat) {
            // in order to repeat a texture, we need to scale the uv's to be larger than the actual source
            if (working_size.width > sprite.source.size.width) {
                uv_scale_x = working_size.width / sprite.source.size.width;
            }

            if (working_size.height > sprite.source.size.height) {
                uv_scale_y = working_size.height / sprite.source.size.height;
            }
        }
        
        tl = SHIZVector2Make(uv_min.x * uv_scale_x, uv_max.y * uv_scale_y);
        br = SHIZVector2Make(uv_max.x * uv_scale_x, uv_min.y * uv_scale_y);
        bl = SHIZVector2Make(uv_min.x * uv_scale_x, uv_min.y * uv_scale_y);
        tr = SHIZVector2Make(uv_max.x * uv_scale_x, uv_max.y * uv_scale_y);
        
        vertices[0].texture_coord = tl;
        vertices[1].texture_coord = br;
        vertices[2].texture_coord = bl;

        vertices[3].texture_coord = tl;
        vertices[4].texture_coord = tr;
        vertices[5].texture_coord = br;

        for (uint i = 0; i < vertex_count; i++) {
            // in order for repeated textures to work (without having to set wrapping modes, and with support for sub-textures)
            // we have to specify the space that uv's are limited to (otherwise a sub-texture with a
            // scaled uv would just end up using part of another subtexture- we don't want that)
            // so this solution will simply "loop over" a scaled uv coordinate so that it is restricted
            // within the dimensions of the expected texture
            vertices[i].texture_coord_min = uv_min;
            vertices[i].texture_coord_max = uv_max;
        }
        
        float const z = SHIZSpriteLayerDefault;
        
        shiz_gfx_render_quad(vertices, SHIZVector3Make(origin.x, origin.y, z),
                             angle, image.texture_id);
    }
}

SHIZSize shiz_measure_sprite_text(SHIZSpriteFont const font, const char* text, SHIZSize const bounds, SHIZVector2 const scale, float const spread) {
    SHIZSpriteFontMeasurement measurement = _shiz_measure_sprite_text(font, text, bounds, scale, spread);

    return measurement.size;
}

static SHIZSpriteFontMeasurement _shiz_measure_sprite_text(SHIZSpriteFont const font, const char* text, SHIZSize const bounds, SHIZVector2 const scale, float const spread) {
    SHIZSpriteFontMeasurement measurement;

    measurement.constrain_index = -1; // no truncation

    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    SHIZVector2 origin = SHIZVector2Zero;
    SHIZVector2 character_origin = origin;

    measurement.character_size = SHIZSizeMake(character_sprite.source.size.width * scale.x,
                                              character_sprite.source.size.height * scale.y);

    measurement.perceived_character_size = SHIZSizeMake(measurement.character_size.width * spread,
                                                        measurement.character_size.height);

    measurement.constrain_horizontally = bounds.width != SHIZSpriteFontSizeToFit.width;
    measurement.constrain_vertically = bounds.height != SHIZSpriteFontSizeToFit.height;

    measurement.max_characters_per_line = floor(bounds.width / measurement.perceived_character_size.width);
    measurement.max_lines_in_bounds = floor(bounds.height / measurement.perceived_character_size.height);

    float const line_height = measurement.perceived_character_size.height;
    
    uint text_index = 0;
    uint line_character_count = 0;
    uint line_index = 0;
    
    while (*text) {
        char const character = *text;

        text += _shiz_get_char_size(character);
        
        bool const break_line_explicit = character == '\n';
        bool const break_line_required = (measurement.constrain_horizontally &&
                                          line_character_count >= measurement.max_characters_per_line);

        if (break_line_explicit || break_line_required) {
            line_character_count = 0;
            line_index += 1;

            measurement.line_size[line_index].width = character_origin.x;
            measurement.line_size[line_index].height = line_height;
            
            character_origin.x = origin.x;
            character_origin.y -= measurement.perceived_character_size.height;
            
            if (line_index > SHIZSpriteFontMaxLines) {
                // this is bad
                break;
            }
            
            if (break_line_explicit) {
                // break early- an explicit linebreak should not be rendered
                continue;
            }
        }

        int character_index = character - font.table.offset;

        if (character_index < 0 ||
            character_index > font.table.columns * font.table.rows) {
            character_index = -1;
        }

        if (character_index != -1) {
            if (measurement.constrain_vertically) {
                if (line_index + 1 > measurement.max_lines_in_bounds) {
                    measurement.constrain_index = text_index - 1; // it was actually the previous character that caused a linebreak

                    break;
                }
            }
        }
        
        // leave a space even if the character was not found
        line_character_count += 1;
        
        character_origin.x += measurement.perceived_character_size.width;

        measurement.line_size[line_index].width = character_origin.x;
        measurement.line_size[line_index].height = line_height;
        
        if (character_origin.x > measurement.size.width) {
            // use the widest occurring width
            measurement.size.width = character_origin.x;
        }
        
        text_index += 1;
    }
    
    measurement.line_count = line_index + 1;
    measurement.size.height = measurement.line_count * line_height;

    return measurement;
}

SHIZSize shiz_draw_sprite_text(SHIZSpriteFont const font, const char* text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment) {
    return shiz_draw_sprite_text_ex(font, text, origin, alignment,
                                    SHIZSpriteFontSizeToFit,
                                    SHIZSpriteFontScaleDefault,
                                    SHIZSpriteNoTint,
                                    SHIZSpriteFontSpreadNormal);
}

SHIZSize shiz_draw_sprite_text_ex(SHIZSpriteFont const font, const char* text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment, SHIZSize const bounds, SHIZVector2 const scale, SHIZColor const tint, float const spread) {
    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    SHIZSpriteFontMeasurement const measurement = _shiz_measure_sprite_text(font, text, bounds, scale, spread);

    uint text_index = 0;
    uint line_index = 0;
    uint line_character_count = 0;

    uint const truncation_length = 3;
    char const truncation_character = '.';
    
    SHIZVector2 character_origin = origin;
    
    if ((alignment & SHIZSpriteFontAlignmentLeft) == SHIZSpriteFontAlignmentLeft) {
        // intenionally left blank; no operation necessary
    } else if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
        character_origin.x -= measurement.line_size[line_index].width / 2;
    } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
        character_origin.x -= measurement.line_size[line_index].width;
    }
    
    if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
        // intenionally left blank; no operation necessary
    } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
        character_origin.y += measurement.size.height / 2;
    } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
        character_origin.y += measurement.size.height;
    }
    
    while (*text) {
        // tail truncation
        bool const should_truncate = measurement.constrain_index != -1 && (text_index > measurement.constrain_index - truncation_length);
        bool const should_break_from_truncation = measurement.constrain_index == text_index;

        char const character = should_truncate ? truncation_character : *text;
        
        text += _shiz_get_char_size(character);
        
        bool const break_line_explicit = character == '\n';
        bool const break_line_required = (measurement.constrain_horizontally &&
                                          line_character_count >= measurement.max_characters_per_line);
        
        if (break_line_explicit || break_line_required) {
            line_index += 1;
            line_character_count = 0;
            
            SHIZSize const line_size = measurement.line_size[line_index];
            
            character_origin.x = origin.x;
            character_origin.y -= line_size.height;
            
            if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
                character_origin.x -= line_size.width / 2;
            } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
                character_origin.x -= line_size.width;
            }
            
            if (line_index > SHIZSpriteFontMaxLines) {
                // this is bad
                break;
            }
            
            if (break_line_explicit) {
                // break early- an explicit linebreak should not be rendered
                continue;
            }
        }
        
        int character_index = character - font.table.offset;
        
        if (character_index < 0 ||
            character_index > font.table.columns * font.table.rows) {
            character_index = -1;
        }
        
        if (character_index != -1) {
            uint const character_row = (int)(character_index / (int)font.table.columns);
            uint const character_column = character_index % (int)font.table.columns;

            character_sprite.source.origin.x = font.sprite.source.origin.x + (font.character.width * character_column);
            character_sprite.source.origin.y = font.sprite.source.origin.y + (font.character.height * character_row);

            shiz_draw_sprite_ex(character_sprite, character_origin, measurement.character_size,
                                SHIZSpriteAnchorTopLeft, SHIZSpriteNoAngle, tint, SHIZSpriteNoRepeat);
        }
        
        // leave a space even if the character was not found and drawn
        line_character_count += 1;
        
        character_origin.x += measurement.perceived_character_size.width;

        if (should_break_from_truncation) {
            break;
        }
        
        text_index += 1;
    }

    return measurement.size;
}

static SHIZViewport _shiz_get_viewport(void) {
    SHIZViewport viewport = SHIZViewportDefault;
    
    viewport.screen = context.preferred_screen_size;
    viewport.framebuffer = _shiz_glfw_get_framebuffer_size();
    viewport.scale = _shiz_glfw_get_pixel_scale();
    
    return viewport;
}

static SHIZSize _shiz_glfw_get_window_size() {
    int window_width;
    int window_height;
    
    glfwGetWindowSize(context.window, &window_width, &window_height);
    
    return SHIZSizeMake(window_width, window_height);
}

static SHIZSize _shiz_glfw_get_framebuffer_size() {
    int framebuffer_width;
    int framebuffer_height;
    
    // determine pixel size of the framebuffer for the window
    // this size is not necesarilly equal to the size of the window, as some
    // platforms may increase the pixel count (e.g. doubling on retina screens)
    glfwGetFramebufferSize(context.window, &framebuffer_width, &framebuffer_height);
    
    return SHIZSizeMake(framebuffer_width, framebuffer_height);
}

static float _shiz_glfw_get_pixel_scale() {
    SHIZSize const framebuffer = _shiz_glfw_get_framebuffer_size();
    SHIZSize const window = _shiz_glfw_get_window_size();
    
    return (framebuffer.width + framebuffer.height) /
    (window.width + window.height);
}

static void _shiz_intro(void) {
    printf("  __|  |  | _ _| __  /  __|   \\ |\n");
    printf("\\__ \\  __ |   |     /   _|   .  |\n");
    printf("____/ _| _| ___| ____| ___| _|\\_|\n\n");
    printf(" SHIZEN %d.%d.%d / %s\n",
           SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
           SHIZEN_VERSION_NAME);
    printf(" Copyright (c) 2016 Jacob Hauberg Hansen\n\n");

    printf("> OPENGL VERSION:  %s (GLSL %s)\n",
           glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("> OPENGL RENDERER: %s\n",
           glGetString(GL_RENDERER));
    printf("> OPENGL VENDOR:   %s\n\n",
           glGetString(GL_VENDOR));
}

static bool _shiz_can_run(void) {
    int major;
    int minor;
    
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    if (major < SHIZ_MIN_OPENGL_VERSION_MAJOR ||
        (major == SHIZ_MIN_OPENGL_VERSION_MAJOR &&
         minor < SHIZ_MIN_OPENGL_VERSION_MINOR)) {
        return false;
    }
    
    return true;
}

#ifdef SHIZ_DEBUG
static void _shiz_debug_process_errors() {
    GLenum error;
    
    while ((error = glGetError()) != GL_NO_ERROR) {
        shiz_io_error_context("OPENGL", "%d", error);
    }
}
#endif

static void _shiz_glfw_error_callback(int error, const char* description) {
    shiz_io_error_context("GLFW", "%d %s", error, description);
}

static void _shiz_glfw_window_close_callback(GLFWwindow* window) {
    (void)window;

    context.should_finish = true;
}

static void _shiz_glfw_window_focus_callback(GLFWwindow* window, int focused) {
    (void)window;

    context.is_focused = focused;
}

static void _shiz_glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    (void)width;
    (void)height;

    shiz_gfx_set_viewport(_shiz_get_viewport());
}
