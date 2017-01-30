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

#include "internal.h"
#include "gfx.h"
#include "res.h"
#include "io.h"

#define SHIZ_MIN_OPENGL_VERSION_MAJOR 3
#define SHIZ_MIN_OPENGL_VERSION_MINOR 3

typedef struct SHIZGraphicsContext {
    /** Determines whether the context has been initialized */
    bool is_initialized;
    /** Determines whether the context has focus */
    bool is_focused;
    /** Determines whether a shutdown should be initiated */
    bool should_finish;
    /** The preferred screen size; the display may be boxed if necessary */
    SHIZSize preferred_screen_size;
    /** A reference to the current window */
    GLFWwindow * window;
} SHIZGraphicsContext;

const SHIZWindowSettings SHIZWindowSettingsDefault = {
    .title = "SHIZEN",
    .fullscreen = false,
    .vsync = true,
    .size = { 320, 240 }
};

static SHIZViewport const SHIZViewportDefault = {
    .framebuffer = {
        .width = 0,
        .height = 0
    },
    .screen = {
        .width = 0,
        .height = 0
    },
    .offset = {
        .width = 0,
        .height = 0
    },
    .scale = 1,
    .is_fullscreen = false
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

static SHIZGraphicsContext _context;

static void
key_callback(GLFWwindow * const window, int key, int scancode, int action, int mods) {
    (void)scancode;
    
    if ((key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        _context.should_finish = true;
    } else if ((mods == GLFW_MOD_ALT && key == GLFW_KEY_ENTER) && action == GLFW_RELEASE) {
        _shiz_glfw_toggle_windowed(window);
    }
#ifdef SHIZ_DEBUG
    else if ((key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        shiz_debug_toggle_enabled();
    }

    if (shiz_debug_is_enabled()) {
        if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_1) && action == GLFW_RELEASE) {
            shiz_debug_toggle_draw_shapes();
        } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_2) && action == GLFW_RELEASE) {
            shiz_debug_toggle_draw_events();
        } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_MINUS) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            shiz_set_time_scale(shiz_get_time_scale() - 0.1);
        } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_EQUAL) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            shiz_set_time_scale(shiz_get_time_scale() + 0.1);
        }
    }
#endif
}

bool
shiz_startup(SHIZWindowSettings const settings) {
    if (_context.is_initialized) {
        return true;
    }

    _context.preferred_screen_size = settings.size;
    
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
    
    _context.is_initialized = true;

    shiz_time_reset();
    
#ifdef SHIZ_DEBUG
    if (!shiz_debug_init()) {
        shiz_io_error("SHIZEN could not initialize a debugging state");
        
        return false;
    }
#endif
    
    return true;
}

bool
shiz_shutdown() {
    if (!_context.is_initialized) {
        return false;
    }

    if (!shiz_gfx_kill()) {
        return false;
    }

#ifdef SHIZ_DEBUG
    if (!shiz_debug_kill()) {
        return false;
    }
#endif
    
    shiz_res_unload_all();

    glfwTerminate();
    
    _context.is_initialized = false;
    
    return true;
}

void
shiz_request_finish() {
    _context.should_finish = true;
}

bool
shiz_should_finish() {
    return _context.should_finish;
}

unsigned int
shiz_load(const char * const filename) {
    return shiz_res_load(shiz_res_get_type(filename), filename);
}

bool
shiz_unload(unsigned int const resource_id) {
    return shiz_res_unload(resource_id);
}

SHIZSprite
shiz_load_sprite(const char * const filename) {
    unsigned int const resource_id = shiz_load(filename);

    if (resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }

    return shiz_get_sprite(resource_id);
}

SHIZSprite
shiz_load_sprite_src(const char * const filename, SHIZRect const source) {
    SHIZSprite sprite = shiz_load_sprite(filename);

    if (sprite.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }

    return shiz_get_sprite_src(sprite.resource_id, source);
}


SHIZSprite
shiz_get_sprite(unsigned int const resource_id) {
    SHIZResourceImage const image = shiz_res_get_image(resource_id);

    if (image.resource_id == SHIZResourceInvalid) {
        return SHIZSpriteEmpty;
    }
    
    SHIZRect const source = SHIZRectMake(SHIZVector2Zero,
                                         SHIZSizeMake(image.width, image.height));

    return shiz_get_sprite_src(resource_id, source);
}

SHIZSprite
shiz_get_sprite_src(unsigned int const resource_id, SHIZRect const source) {
    SHIZSprite sprite;

    sprite.resource_id = resource_id;
    sprite.source = source;

    return sprite;
}

SHIZSpriteFont
shiz_load_sprite_font(const char * const filename, SHIZSize const character) {
    SHIZSprite const sprite = shiz_load_sprite(filename);
    
    return shiz_get_sprite_font(sprite, character);
}

SHIZSpriteFont
shiz_load_sprite_font_ex(const char * const filename,
                         SHIZSize const character,
                         SHIZSpriteFontTable const table) {
    SHIZSpriteFont const spritefont = shiz_load_sprite_font(filename, character);
    
    return shiz_get_sprite_font_ex(spritefont.sprite, spritefont.character, table);
}

SHIZSpriteFont
shiz_get_sprite_font(SHIZSprite const sprite, SHIZSize const character) {
    SHIZSpriteFontTable table;

    table.columns = sprite.source.size.width / character.width;
    table.rows = sprite.source.size.height / character.height;
    table.offset = 0;

    return shiz_get_sprite_font_ex(sprite, character, table);
}

SHIZSpriteFont
shiz_get_sprite_font_ex(SHIZSprite const sprite,
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

static bool
_shiz_glfw_create_window(SHIZWindowSettings const settings) {
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
            
            _context.window = glfwCreateWindow(display_width, display_height,
                                               settings.title, glfwGetPrimaryMonitor(), NULL);
            
            // prefer centered window if initially fullscreen;
            // otherwise let the OS determine window placement
            _shiz_glfw_window_position.x = (display_width / 2) - (settings.size.width / 2);
            _shiz_glfw_window_position.y = (display_height / 2) - (settings.size.height / 2);
        }
    } else {
        _context.window = glfwCreateWindow(settings.size.width, settings.size.height,
                                               settings.title, NULL, NULL);
    }
    
    if (!_context.window) {
        return false;
    }
    
    glfwSetWindowCloseCallback(_context.window, _shiz_glfw_window_close_callback);
    glfwSetWindowFocusCallback(_context.window, _shiz_glfw_window_focus_callback);
    
    glfwSetFramebufferSizeCallback(_context.window, _shiz_glfw_framebuffer_size_callback);
    
    glfwSetKeyCallback(_context.window, key_callback);
    
    glfwMakeContextCurrent(_context.window);
    glfwSwapInterval(settings.vsync ? 1 : 0);
    
    return true;
}

static
SHIZViewport _shiz_get_viewport(void) {
    SHIZViewport viewport = SHIZViewportDefault;

    viewport.screen = _shiz_get_preferred_screen_size();
    viewport.framebuffer = _shiz_glfw_get_framebuffer_size();
    viewport.scale = _shiz_glfw_get_pixel_scale();

    if (glfwGetWindowMonitor(_context.window)) {
        viewport.is_fullscreen = true;
    }

    return viewport;
}

static
SHIZSize _shiz_glfw_get_window_size() {
    int window_width;
    int window_height;

    glfwGetWindowSize(_context.window, &window_width, &window_height);
    
    return SHIZSizeMake(window_width, window_height);
}

static
SHIZSize _shiz_glfw_get_framebuffer_size() {
    int framebuffer_width;
    int framebuffer_height;
    
    // determine pixel size of the framebuffer for the window
    // this size is not necesarilly equal to the size of the window, as some
    // platforms may increase the pixel count (e.g. doubling on retina screens)
    glfwGetFramebufferSize(_context.window, &framebuffer_width, &framebuffer_height);
    
    return SHIZSizeMake(framebuffer_width, framebuffer_height);
}

static float
_shiz_glfw_get_pixel_scale() {
    SHIZSize const framebuffer = _shiz_glfw_get_framebuffer_size();
    SHIZSize const window = _shiz_glfw_get_window_size();
    
    return (framebuffer.width + framebuffer.height) / (window.width + window.height);
}

static void
_shiz_intro(void) {
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

static bool
_shiz_can_run(void) {
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

static void
_shiz_glfw_toggle_windowed(GLFWwindow * const window) {
    bool const is_currently_fullscreen = glfwGetWindowMonitor(window) != NULL;

    int window_position_x = _shiz_glfw_window_position.x;
    int window_position_y = _shiz_glfw_window_position.y;
    
    if (is_currently_fullscreen) {
        // go windowed
        glfwSetWindowMonitor(window, NULL,
                             window_position_x, window_position_y,
                             _context.preferred_screen_size.width,
                             _context.preferred_screen_size.height,
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

static void
_shiz_glfw_error_callback(int error, const char * const description) {
    shiz_io_error_context("GLFW", "%d %s", error, description);
}

static void
_shiz_glfw_window_close_callback(GLFWwindow * const window) {
    (void)window;

    _context.should_finish = true;
}

static void
_shiz_glfw_window_focus_callback(GLFWwindow * const window, int focused) {
    (void)window;

    _context.is_focused = focused;
}

static void
_shiz_glfw_framebuffer_size_callback(GLFWwindow * const window, int width, int height) {
    (void)window;
    (void)width;
    (void)height;

    SHIZViewport const viewport = _shiz_get_viewport();

    shiz_gfx_set_viewport(viewport);
}

SHIZSize _shiz_get_preferred_screen_size() {
    return _context.preferred_screen_size;
}

void _shiz_present_frame() {
    glfwSwapBuffers(_context.window);
    glfwPollEvents();
}
