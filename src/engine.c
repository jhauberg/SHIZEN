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

#include <SHIZEN/engine.h>

#include <stdio.h>
#include <math.h>

#include "internal.h"
#include "gfx.h"
#include "res.h"
#include "io.h"

#define SHIZ_MIN_OPENGL_VERSION_MAJOR 3
#define SHIZ_MIN_OPENGL_VERSION_MINOR 3

const SHIZWindowSettings SHIZWindowSettingsDefault = {
    .title = "SHIZEN",
    .fullscreen = false,
    .vsync = true,
    .size = { 320, 240 }
};

static void _shiz_glfw_error_callback(int error, const char * const description);

static void _shiz_glfw_window_close_callback(GLFWwindow * const window);
static void _shiz_glfw_window_focus_callback(GLFWwindow * const window, int focused);

static void _shiz_glfw_framebuffer_size_callback(GLFWwindow * const window, int width, int height);
static bool _shiz_glfw_create_window(SHIZWindowSettings const settings);

static SHIZSize _shiz_glfw_get_window_size(void);
static SHIZSize _shiz_glfw_get_framebuffer_size(void);

static void _shiz_glfw_toggle_windowed(GLFWwindow * const window);

static SHIZViewport _shiz_get_viewport(void);

static float _shiz_glfw_get_pixel_scale(void);

static void _shiz_intro(void);
static bool _shiz_can_run(void);

static SHIZVector2 _shiz_glfw_window_position;

static SHIZTimeLine _timeline;

static double const maximum_frame_time = 1.0 / 4; // 4 frames per second

static double _time_previous = 0;
static double _time_lag = 0;

SHIZGraphicsContext shiz_context;

#ifdef SHIZ_DEBUG
SHIZSpriteFont shiz_debug_font;
SHIZDebugContext shiz_debug_context;
#endif

static void key_callback(GLFWwindow * const window, int key, int scancode, int action, int mods) {
    (void)scancode;
    
    if ((key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        shiz_context.should_finish = true;
    } else if ((mods == GLFW_MOD_ALT && key == GLFW_KEY_ENTER) && action == GLFW_RELEASE) {
        _shiz_glfw_toggle_windowed(window);
    }
#ifdef SHIZ_DEBUG
    else if ((key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        shiz_debug_context.is_enabled = !shiz_debug_context.is_enabled;
    } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_1) && action == GLFW_RELEASE) {
        if (shiz_debug_context.is_enabled) {
            shiz_debug_context.draw_sprite_shape = !shiz_debug_context.draw_sprite_shape;
        }
    } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_2) && action == GLFW_RELEASE) {
        if (shiz_debug_context.is_enabled) {
            shiz_debug_context.draw_events = !shiz_debug_context.draw_events;
        }
    } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_MINUS) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (shiz_debug_context.is_enabled) {
            _timeline.scale -= 0.1f;
        }
    } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_EQUAL) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (shiz_debug_context.is_enabled) {
            _timeline.scale += 0.1f;
        }
    }
#endif
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
        GLFWmonitor * monitor = glfwGetPrimaryMonitor();

        if (monitor) {
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);

            int const display_width = mode->width;
            int const display_height = mode->height;
            
            shiz_context.window = glfwCreateWindow(display_width, display_height,
                                                   settings.title, glfwGetPrimaryMonitor(), NULL);
            
            // prefer centered window if initially fullscreen;
            // otherwise let the OS determine window placement
            _shiz_glfw_window_position.x = (display_width / 2) - (settings.size.width / 2);
            _shiz_glfw_window_position.y = (display_height / 2) - (settings.size.height / 2);
        }
    } else {
        shiz_context.window = glfwCreateWindow(settings.size.width, settings.size.height,
                                               settings.title, NULL, NULL);
    }

    if (!shiz_context.window) {
        return false;
    }

    glfwSetWindowCloseCallback(shiz_context.window, _shiz_glfw_window_close_callback);
    glfwSetWindowFocusCallback(shiz_context.window, _shiz_glfw_window_focus_callback);

    glfwSetFramebufferSizeCallback(shiz_context.window, _shiz_glfw_framebuffer_size_callback);

    glfwSetKeyCallback(shiz_context.window, key_callback);

    glfwMakeContextCurrent(shiz_context.window);
    glfwSwapInterval(settings.vsync ? 1 : 0);

    return true;
}

bool shiz_startup(SHIZWindowSettings const settings) {
    if (shiz_context.is_initialized) {
        return true;
    }

    shiz_context.preferred_screen_size = settings.size;
    
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
    
    shiz_context.is_initialized = true;

    _timeline.time = 0;
    _timeline.time_step = 0;
    _timeline.scale = 1;

    glfwSetTime(_timeline.time);
    
    _time_previous = glfwGetTime();

#ifdef SHIZ_DEBUG
    shiz_debug_context.is_enabled = true;

    if (shiz_res_debug_load_font()) {
        SHIZSprite sprite = shiz_get_sprite(shiz_res_debug_get_font());
        SHIZSpriteFont spritefont = shiz_get_sprite_font(sprite, SHIZSizeMake(8, 8));

        shiz_debug_font = spritefont;
        shiz_debug_font.table.offset = 32;
    }
#endif
    
    return true;
}

bool shiz_shutdown() {
    if (!shiz_context.is_initialized) {
        return false;
    }

    if (!shiz_gfx_kill()) {
        return false;
    }

    shiz_res_unload_all();

    glfwTerminate();
    
    shiz_context.is_initialized = false;
    
    return true;
}

void shiz_request_finish() {
    shiz_context.should_finish = true;
}

bool shiz_should_finish() {
    return shiz_context.should_finish;
}

void shiz_ticking_begin(void) {
    double const time = glfwGetTime();
    double time_elapsed = time - _time_previous;
    
    if (time_elapsed > maximum_frame_time) {
        time_elapsed = maximum_frame_time;
    }
    
    _time_lag += fabs(time_elapsed * _timeline.scale);
    _time_previous = time;
}

float shiz_ticking_end(void) {
    return _time_lag / _timeline.time_step;
}

bool shiz_tick(uint const frequency) {
    _timeline.time_step = 1.0 / frequency;

    if (_time_lag >= _timeline.time_step) {
        _time_lag -= _timeline.time_step;

        _timeline.time += _timeline.time_step * shiz_get_time_direction();
        
        return true;
    }
    
    return false;
}

double shiz_get_time() {
    return _timeline.time;
}

float shiz_get_time_direction() {
    if (_timeline.scale > 0) {
        return 1;
    } else if (_timeline.scale < 0) {
        return -1;
    }

    return 0;
}

double shiz_get_tick_rate() {
    return _timeline.time_step;
}

uint shiz_load(const char * const filename) {
    return shiz_res_load(shiz_res_get_type(filename), filename);
}

bool shiz_unload(uint const resource_id) {
    return shiz_res_unload(resource_id);
}

SHIZSprite shiz_load_sprite(const char * const filename) {
    uint const resource_id = shiz_load(filename);
    
    return shiz_get_sprite(resource_id);
}

SHIZSprite shiz_load_sprite_src(const char * const filename, SHIZRect const source) {
    SHIZSprite sprite = shiz_load_sprite(filename);
    
    return shiz_get_sprite_src(sprite.resource_id, source);
}

SHIZSprite shiz_get_sprite(uint const resource_id) {
    SHIZResourceImage const image = shiz_res_get_image(resource_id);

    SHIZRect const source = SHIZRectMake(SHIZVector2Zero,
                                         SHIZSizeMake(image.width, image.height));

    return shiz_get_sprite_src(resource_id, source);
}

SHIZSprite shiz_get_sprite_src(uint const resource_id, SHIZRect const source) {
    SHIZSprite sprite;

    sprite.resource_id = resource_id;
    sprite.source = source;

    return sprite;
}

SHIZSpriteFont shiz_load_sprite_font(const char * const filename, SHIZSize const character) {
    SHIZSprite const sprite = shiz_load_sprite(filename);
    
    return shiz_get_sprite_font(sprite, character);
}

SHIZSpriteFont shiz_load_sprite_font_ex(const char * const filename,
                                        SHIZSize const character,
                                        SHIZSpriteFontTable const table) {
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

SHIZSpriteFont shiz_get_sprite_font_ex(SHIZSprite const sprite,
                                       SHIZSize const character,
                                       SHIZSpriteFontTable const table) {
    SHIZSpriteFont spritefont;
    
    spritefont.sprite = sprite;
    spritefont.character = character;
    spritefont.table = table;
    // default to skip whitespaces; this will reduce the number of sprites drawn
    spritefont.includes_whitespace = false;
    
    return spritefont;
}

static SHIZViewport _shiz_get_viewport(void) {
    SHIZViewport viewport = SHIZViewportDefault;

    viewport.screen = shiz_context.preferred_screen_size;
    viewport.framebuffer = _shiz_glfw_get_framebuffer_size();
    viewport.scale = _shiz_glfw_get_pixel_scale();

    if (glfwGetWindowMonitor(shiz_context.window)) {
        viewport.is_fullscreen = true;
    }

    return viewport;
}

static SHIZSize _shiz_glfw_get_window_size() {
    int window_width;
    int window_height;

    glfwGetWindowSize(shiz_context.window, &window_width, &window_height);
    
    return SHIZSizeMake(window_width, window_height);
}

static SHIZSize _shiz_glfw_get_framebuffer_size() {
    int framebuffer_width;
    int framebuffer_height;
    
    // determine pixel size of the framebuffer for the window
    // this size is not necesarilly equal to the size of the window, as some
    // platforms may increase the pixel count (e.g. doubling on retina screens)
    glfwGetFramebufferSize(shiz_context.window, &framebuffer_width, &framebuffer_height);
    
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
    printf(" SHIZEN %d.%d.%d / %s (built %s, %s)\n",
           SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
           SHIZEN_VERSION_NAME, __DATE__, __TIME__);
    printf(" Copyright (c) 2017 Jacob Hauberg Hansen\n\n");

    printf(" OPENGL VERSION:  %s (GLSL %s)\n",
           glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf(" OPENGL RENDERER: %s\n",
           glGetString(GL_RENDERER));
    printf(" OPENGL VENDOR:   %s\n\n",
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

static void _shiz_glfw_toggle_windowed(GLFWwindow * const window) {
    bool const is_currently_fullscreen = glfwGetWindowMonitor(window) != NULL;

    int window_position_x = _shiz_glfw_window_position.x;
    int window_position_y = _shiz_glfw_window_position.y;
    
    if (is_currently_fullscreen) {
        // go windowed
        glfwSetWindowMonitor(window, NULL,
                             window_position_x, window_position_y,
                             shiz_context.preferred_screen_size.width,
                             shiz_context.preferred_screen_size.height,
                             0);
    } else {
        // go fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        if (monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            glfwGetWindowPos(window,
                             &window_position_x,
                             &window_position_y);
         
            _shiz_glfw_window_position.x = window_position_x;
            _shiz_glfw_window_position.y = window_position_y;
            
            glfwSetWindowMonitor(window, monitor, 0, 0,
                                 mode->width, mode->height,
                                 mode->refreshRate);
        }
    }
}

static void _shiz_glfw_error_callback(int error, const char * const description) {
    shiz_io_error_context("GLFW", "%d %s", error, description);
}

static void _shiz_glfw_window_close_callback(GLFWwindow * const window) {
    (void)window;

    shiz_context.should_finish = true;
}

static void _shiz_glfw_window_focus_callback(GLFWwindow * const window, int focused) {
    (void)window;

    shiz_context.is_focused = focused;
}

static void _shiz_glfw_framebuffer_size_callback(GLFWwindow * const window, int width, int height) {
    (void)window;
    (void)width;
    (void)height;

    SHIZViewport const viewport = _shiz_get_viewport();

    shiz_gfx_set_viewport(viewport);
}
