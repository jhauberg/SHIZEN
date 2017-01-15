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

const SHIZWindowSettings SHIZWindowSettingsDefault = {
    "SHIZEN",    /* title */
    false,       /* fullscreen */
    true,        /* vsync */
    { 320, 240 } /* preferred screen size */
};

static void _shiz_glfw_error_callback(int error, const char* description);

static void _shiz_glfw_window_close_callback(GLFWwindow* window);
static void _shiz_glfw_window_focus_callback(GLFWwindow* window, int focused);

static void _shiz_glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
static bool _shiz_glfw_create_window(SHIZWindowSettings const settings);

static SHIZSize _shiz_glfw_get_window_size(void);
static SHIZSize _shiz_glfw_get_framebuffer_size(void);

static void _shiz_glfw_toggle_windowed(GLFWwindow *window);

static SHIZViewport _shiz_get_viewport(void);

static float _shiz_glfw_get_pixel_scale(void);

static void _shiz_intro(void);
static bool _shiz_can_run(void);

static void _shiz_draw_rect(SHIZRect const rect, SHIZColor const color, bool const fill);

static SHIZSpriteFontMeasurement _shiz_measure_sprite_text(SHIZSpriteFont const font, const char* text, SHIZSize const bounds, SHIZSpriteFontAttributes const attributes);

#ifdef SHIZ_DEBUG
static void _shiz_debug_process_errors(void);

static SHIZSpriteFont _shiz_debug_font;
static char _shiz_debug_font_buffer[128];
#endif

static SHIZGraphicsContext context;
static SHIZTimeLine timeline;

static double const maximum_frame_time = 1.0 / 4; // 4 frames per second

static double time_previous = 0;
static double time_lag = 0;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;

    if ((key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        context.should_finish = true;
    } else if ((key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT) && action == GLFW_RELEASE) {
        _shiz_glfw_toggle_windowed(window);
    }
}

static bool _shiz_glfw_create_window(SHIZWindowSettings const settings) {
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SHIZ_MIN_OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SHIZ_MIN_OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if (settings.fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();

        if (monitor) {
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);

            context.window = glfwCreateWindow(mode->width, mode->height,
                                              settings.title, glfwGetPrimaryMonitor(), NULL);
        }
    } else {
        context.window = glfwCreateWindow(settings.size.width, settings.size.height,
                                          settings.title, NULL, NULL);
    }

    if (!context.window) {
        return false;
    }

    glfwSetWindowCloseCallback(context.window, _shiz_glfw_window_close_callback);
    glfwSetWindowFocusCallback(context.window, _shiz_glfw_window_focus_callback);

    glfwSetFramebufferSizeCallback(context.window, _shiz_glfw_framebuffer_size_callback);

    glfwSetKeyCallback(context.window, key_callback);

    glfwMakeContextCurrent(context.window);
    glfwSwapInterval(settings.vsync ? 1 : 0);

    return true;
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

    if (!_shiz_glfw_create_window(settings)) {
        shiz_io_error_context("GLFW", "(%s) failed to create window", glfwGetVersionString());

        return false;
    }

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

    timeline.time = 0;
    timeline.time_step = 0;
    timeline.scale = 1;
    
    time_previous = glfwGetTime();

#ifdef SHIZ_DEBUG
    if (shiz_res_debug_load_font()) {
        SHIZSprite sprite = shiz_get_sprite(shiz_res_debug_get_font());
        SHIZSpriteFont spritefont = shiz_get_sprite_font(sprite, SHIZSizeMake(8, 8));

        _shiz_debug_font = spritefont;
        _shiz_debug_font.table.offset = 32;
    }
#endif
    
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

void shiz_ticking_begin(void) {
    double const time = glfwGetTime();
    double time_elapsed = time - time_previous;
    
    if (time_elapsed > maximum_frame_time) {
        time_elapsed = maximum_frame_time;
    }
    
    time_lag += time_elapsed * timeline.scale;
    time_previous = time;
}

float shiz_ticking_end(void) {
    return time_lag / timeline.time_step;
}

bool shiz_tick(uint const frequency) {
    timeline.time_step = 1.0 / frequency;
    
    if (time_lag >= timeline.time_step) {
        time_lag -= timeline.time_step;

        timeline.time += timeline.time_step;
        
        return true;
    }
    
    return false;
}

double shiz_get_time() {
    return timeline.time;
}

double shiz_get_tick_rate() {
    return timeline.time_step;
}

uint shiz_load(const char *filename) {
    return shiz_res_load(shiz_res_get_type(filename), filename);
}

bool shiz_unload(uint const resource_id) {
    return shiz_res_unload(resource_id);
}

SHIZSprite shiz_load_sprite(const char *filename) {
    uint const resource_id = shiz_load(filename);
    
    return shiz_get_sprite(resource_id);
}

SHIZSprite shiz_load_sprite_src(const char *filename, SHIZRect source) {
    SHIZSprite sprite = shiz_load_sprite(filename);
    
    return shiz_get_sprite_src(sprite.resource_id, source);
}

SHIZSprite shiz_get_sprite(uint const resource_id) {
    SHIZResourceImage const image = shiz_res_get_image(resource_id);

    SHIZRect const source = SHIZRectMake(SHIZVector2Zero,
                                         SHIZSizeMake(image.width, image.height));

    return shiz_get_sprite_src(resource_id, source);
}

SHIZSprite shiz_get_sprite_src(uint const resource_id, SHIZRect source) {
    SHIZSprite sprite;

    sprite.resource_id = resource_id;
    sprite.source = source;

    return sprite;
}

SHIZSpriteFont shiz_load_sprite_font(const char *filename, SHIZSize const character) {
    SHIZSprite const sprite = shiz_load_sprite(filename);
    
    return shiz_get_sprite_font(sprite, character);
}

SHIZSpriteFont shiz_load_sprite_font_ex(const char *filename, SHIZSize const character, SHIZSpriteFontTable const table) {
    SHIZSpriteFont const spritefont = shiz_load_sprite_font(filename, character);
    
    return shiz_get_sprite_font_ex(spritefont.sprite, spritefont.character, table);
}

SHIZSpriteFont shiz_get_sprite_font(SHIZSprite const sprite, SHIZSize const character) {
    SHIZSpriteFontTable table;

    table.columns = sprite.source.size.width / character.width;
    table.rows = sprite.source.size.height / character.height;
    table.offset = 0;

    return shiz_get_sprite_font_ex(sprite, character, table);
}

SHIZSpriteFont shiz_get_sprite_font_ex(SHIZSprite const sprite, SHIZSize const character, SHIZSpriteFontTable const table) {
    SHIZSpriteFont spritefont;
    
    spritefont.sprite = sprite;
    spritefont.character = character;
    spritefont.table = table;
    spritefont.includes_whitespace = false; // default to skip whitespaces; this will reduce the number of sprites drawn
    
    return spritefont;
}

void shiz_drawing_begin() {
    shiz_gfx_clear();
    shiz_gfx_begin();
}

void shiz_drawing_end() {
#ifdef SHIZ_DEBUG
    uint const margin = 4;

    // drawing debug text prior to ending the frame context will make sure everything is
    // flushed and rendered during *this* frame-
    shiz_draw_sprite_text(_shiz_debug_font,
                          _shiz_debug_font_buffer,
                          SHIZVector2Make(margin, context.preferred_screen_size.height - margin),
                          SHIZSpriteFontAlignmentTop | SHIZSpriteFontAlignmentLeft);
#endif

    shiz_gfx_end();

#ifdef SHIZ_DEBUG
    _shiz_debug_process_errors();

    SHIZViewport const viewport = shiz_gfx_get_viewport();

    sprintf(_shiz_debug_font_buffer,
            "SPEED: %0.1fms/frame\n"
            "       %d (%d / %d / %d)\n"
            "DRAW:  %d (%.0fx%.0f @ %.0fx%.0f)",
            shiz_gfx_debug_get_frame_time(),
            shiz_gfx_debug_get_frames_per_second(),
            shiz_gfx_debug_get_frames_per_second_min(),
            shiz_gfx_debug_get_frames_per_second_avg(),
            shiz_gfx_debug_get_frames_per_second_max(),
            // note that draw count will also include the debug stuff, so in production
            // this count may actually be smaller (likely not significantly smaller, though)
            shiz_gfx_debug_get_draw_count(),
            viewport.screen.width, viewport.screen.height,
            viewport.framebuffer.width, viewport.framebuffer.height);
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

SHIZSize shiz_measure_sprite_text(SHIZSpriteFont const font, const char* text, SHIZSize const bounds, SHIZSpriteFontAttributes const attributes) {
    SHIZSpriteFontMeasurement measurement = _shiz_measure_sprite_text(font, text, bounds, attributes);

    return measurement.size;
}

static SHIZSpriteFontMeasurement _shiz_measure_sprite_text(SHIZSpriteFont const font, const char* text, SHIZSize const bounds, SHIZSpriteFontAttributes const attributes) {
    SHIZSpriteFontMeasurement measurement;

    measurement.constrain_index = -1; // no truncation

    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    measurement.character_size = SHIZSizeMake(character_sprite.source.size.width * attributes.scale.x,
                                              character_sprite.source.size.height * attributes.scale.y);

    measurement.character_size_perceived = SHIZSizeMake((measurement.character_size.width * attributes.character_spread) + attributes.character_padding,
                                                        measurement.character_size.height);

    measurement.constrain_horizontally = bounds.width != SHIZSpriteFontSizeToFit.width;
    measurement.constrain_vertically = bounds.height != SHIZSpriteFontSizeToFit.height;

    measurement.max_characters_per_line = floor(bounds.width / measurement.character_size_perceived.width);
    measurement.max_lines_in_bounds = floor(bounds.height / measurement.character_size_perceived.height);

    float const line_height = measurement.character_size_perceived.height + attributes.line_padding;
    
    uint text_index = 0;
    uint line_index = 0;
    uint line_character_count = 0;
    
    char const whitespace_character = ' ';
    char const newline_character = '\n';
    
    while (*text) {
        char character = *text;

        text += _shiz_get_char_size(character);
        
        bool const break_line_explicit = character == newline_character;
        bool const break_line_required = (measurement.constrain_horizontally &&
                                          line_character_count >= measurement.max_characters_per_line);

        if (break_line_explicit || break_line_required) {
            if (break_line_required && attributes.wrap == SHIZSpriteFontWrapModeWord) {
                // backtrack until finding a whitespace
                while (*text) {
                    text -= _shiz_get_char_size(character);
                    text_index -= 1;
                    
                    character = *text;
                    
                    if (*text == whitespace_character) {
                        break;
                    }
                    
                    line_character_count -= 1;
                }
            }
            
            measurement.line_size[line_index].width = line_character_count * measurement.character_size_perceived.width;
            measurement.line_size[line_index].height = line_height;
            
            line_character_count = 0;
            line_index += 1;
            
            if (line_index > SHIZSpriteFontMaxLines) {
                // this is bad
                break;
            }
            
            continue;
        }

        if (measurement.constrain_vertically) {
            if (line_index + 1 > measurement.max_lines_in_bounds) {
                measurement.constrain_index = text_index - 1; // it was actually the previous character that caused a linebreak

                break;
            }
        }
        
        // leave a space even if the character was not found
        line_character_count += 1;
        
        measurement.line_size[line_index].width = line_character_count * measurement.character_size_perceived.width;
        measurement.line_size[line_index].height = line_height;
        
        text_index += 1;
    }
    
    measurement.line_count = line_index + 1;
    measurement.size.height = measurement.line_count * line_height;

    for (uint i = 0; i < measurement.line_count; i++) {
        if (measurement.line_size[i].width > measurement.size.width) {
            // use the widest occurring line width
            measurement.size.width = measurement.line_size[i].width;
        }
    }
    
    return measurement;
}

SHIZSize shiz_draw_sprite_text(SHIZSpriteFont const font, const char* text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment) {
    return shiz_draw_sprite_text_ex(font, text, origin, alignment,
                                    SHIZSpriteFontSizeToFit,
                                    SHIZSpriteNoTint,
                                    SHIZSpriteFontAttributesDefault);
}

SHIZSize shiz_draw_sprite_text_ex(SHIZSpriteFont const font, const char* text, SHIZVector2 const origin, SHIZSpriteFontAlignment const alignment, SHIZSize const bounds, SHIZColor const tint, SHIZSpriteFontAttributes const attributes) {
    SHIZSprite character_sprite = SHIZSpriteEmpty;

    character_sprite.resource_id = font.sprite.resource_id;
    character_sprite.source = SHIZRectMake(font.sprite.source.origin, font.character);

    SHIZSpriteFontMeasurement const measurement = _shiz_measure_sprite_text(font, text, bounds, attributes);

    uint const truncation_length = 3;
    char const truncation_character = '.';
    char const whitespace_character = ' ';
    char const newline_character = '\n';
    
    SHIZVector2 character_origin = origin;

    if ((alignment & SHIZSpriteFontAlignmentTop) == SHIZSpriteFontAlignmentTop) {
        // intenionally left blank; no operation necessary
    } else if ((alignment & SHIZSpriteFontAlignmentMiddle) == SHIZSpriteFontAlignmentMiddle) {
        character_origin.y += measurement.size.height / 2;
    } else if ((alignment & SHIZSpriteFontAlignmentBottom) == SHIZSpriteFontAlignmentBottom) {
        character_origin.y += measurement.size.height;
    }

    uint text_index = 0;
    
    bool should_break_from_truncation = false;
    
    for (uint line_index = 0; line_index < measurement.line_count; line_index++) {
        SHIZSize line_size = measurement.line_size[line_index];
        uint line_character_count = line_size.width / measurement.character_size_perceived.width;
        
        if ((alignment & SHIZSpriteFontAlignmentCenter) == SHIZSpriteFontAlignmentCenter) {
            character_origin.x -= line_size.width / 2;
        } else if ((alignment & SHIZSpriteFontAlignmentRight) == SHIZSpriteFontAlignmentRight) {
            character_origin.x -= line_size.width;
        }
        
        for (uint character_index = 0; character_index < line_character_count; character_index++) {
            bool const should_truncate = measurement.constrain_index != -1 && (text_index > measurement.constrain_index - truncation_length);
      
            should_break_from_truncation = measurement.constrain_index == text_index;
            
            char character = should_truncate ? truncation_character : text[text_index];

            text_index += 1;
            
            if (character == newline_character) {
                // ignore newlines and just proceed as if this iteration never happened
                character_index--;
                
                continue;
            }
            
            int character_table_index = character - font.table.offset;
            
            if (character_table_index < 0 ||
                character_table_index > font.table.columns * font.table.rows) {
                character_table_index = -1;
            }
                        
            bool const should_skip_leading_whitespace = !font.includes_whitespace && attributes.wrap == SHIZSpriteFontWrapModeWord;
            bool const is_leading_whitespace = character_index == 0 && character == whitespace_character;
            
            bool character_takes_space = true;

            if (is_leading_whitespace && should_skip_leading_whitespace) {
                character_takes_space = false;

                // the index has already been incremented once, so we have to step back by 2
                int const previous_text_index = text_index - 2;

                if (previous_text_index >= 0) {
                    char const previous_character = text[previous_text_index];

                    if (previous_character == newline_character) {
                        // this was an explicit line-break, so the leading whitespace is probably intentional
                        character_takes_space = true;
                    }
                }
            }
      
            if (character_table_index != -1) {
                bool can_draw_character = character != whitespace_character || font.includes_whitespace;
                
                if (can_draw_character) {
                    uint const character_row = (int)(character_table_index / (int)font.table.columns);
                    uint const character_column = character_table_index % (int)font.table.columns;
                    
                    character_sprite.source.origin.x = font.sprite.source.origin.x + (font.character.width * character_column);
                    character_sprite.source.origin.y = font.sprite.source.origin.y + (font.character.height * character_row);
                    
                    shiz_draw_sprite_ex(character_sprite, character_origin, measurement.character_size,
                                        SHIZSpriteAnchorTopLeft, SHIZSpriteNoAngle, tint, SHIZSpriteNoRepeat);
                }
            }
            
            if (character_takes_space) {
                character_origin.x += measurement.character_size_perceived.width;
            }
            
            if (should_break_from_truncation) {
                // we need to break out of everything once we reach the final visible character
                break;
            }
        }
        
        character_origin.x = origin.x;
        character_origin.y -= line_size.height;
        
        if (should_break_from_truncation) {
            break;
        }
    }

    return measurement.size;
}

static SHIZViewport _shiz_get_viewport(void) {
    SHIZViewport viewport = SHIZViewportDefault;

    viewport.screen = context.preferred_screen_size;
    viewport.framebuffer = _shiz_glfw_get_framebuffer_size();
    viewport.scale = _shiz_glfw_get_pixel_scale();

    if (glfwGetWindowMonitor(context.window)) {
        viewport.is_fullscreen = true;
    }

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
    
    return (framebuffer.width + framebuffer.height) / (window.width + window.height);
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

static void _shiz_glfw_toggle_windowed(GLFWwindow *window) {
    bool const is_currently_fullscreen = glfwGetWindowMonitor(window) != NULL;

    if (is_currently_fullscreen) {
        // go windowed
        glfwSetWindowMonitor(window, NULL,
                             0, 0,
                             context.preferred_screen_size.width,
                             context.preferred_screen_size.height,
                             0);
    } else {
        // go fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        if (monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            glfwSetWindowMonitor(window, monitor, 0, 0,
                                 mode->width, mode->height,
                                 mode->refreshRate);
        }
    }
}

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

    SHIZViewport const viewport = _shiz_get_viewport();

    shiz_gfx_set_viewport(viewport);
}
