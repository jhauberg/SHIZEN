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

#include <SHIZEN/shizen.h>

#include <stdio.h> // printf

#include "graphics/gfx.h"

#include "internal.h"
#include "viewport.h"
#include "res.h"
#include "io.h"

#ifdef SHIZ_DEBUG
 #include "debug/debug.h"
#endif

#define SHIZ_MIN_OPENGL_VERSION_MAJOR 3
#define SHIZ_MIN_OPENGL_VERSION_MINOR 3

const SHIZWindowSettings SHIZWindowSettingsDefault = {
    .title = "SHIZEN",
    .description = NULL,
    .fullscreen = false,
    .vsync = true,
    .size = {
        .width = 320,
        .height = 240
    },
    .pixel_size = 1
};

static
void
z_engine__error_callback(int error, char const * description);

static
void
z_engine__window_close_callback(GLFWwindow *);

static
void
z_engine__window_focus_callback(GLFWwindow *, int focused);

static
void
z_engine__key_callback(GLFWwindow * const window,
                       int key, int scancode, int action, int mods);

static
void
z_engine__framebuffer_size_callback(GLFWwindow *,
                                    int width, int height);

static
bool
z_engine__create_window(bool fullscreen,
                        char const * title);

static
void
z_engine__toggle_windowed(GLFWwindow *);

static
bool
z_engine__is_fullscreen(void);

static
f32
z_engine__get_pixel_scale(void);

static
SHIZSize
z_engine__get_window_size(void);

static
SHIZSize
z_engine__get_framebuffer_size(void);

static
SHIZViewport
z_engine__build_viewport(void);

static
bool
z_engine__can_run(void);

static
void
z_engine__intro(char const * description);

static
void
z_engine__intro_gl(void);

static SHIZVector2 _preferred_window_position;

SHIZGraphicsContext _graphics_context = {
    .window = NULL,
    .native_size = {
        .width = 0,
        .height = 0
    },
    .display_size = {
        .width = 0,
        .height = 0
    },
    .swap_interval = 0,
    .pixel_size = 1,
    .is_initialized = false,
    .is_focused = false,
    .should_finish = false
};

bool
z_startup(SHIZWindowSettings const settings)
{
    if (_graphics_context.is_initialized) {
        return false;
    }

    z_engine__intro(settings.description);
    
    _graphics_context.native_size = settings.size;
    _graphics_context.pixel_size = 1;
    _graphics_context.swap_interval = settings.vsync ? 1 : 0;
    
    if (settings.pixel_size > 0) {
        _graphics_context.pixel_size = settings.pixel_size;
    } else {
        z_io__warning("SHIZEN does not support a pixel-size of 0; defaulting to 1");
    }
    
    _graphics_context.display_size =
        SHIZSizeMake(_graphics_context.native_size.width * _graphics_context.pixel_size,
                     _graphics_context.native_size.height * _graphics_context.pixel_size);
    
    glfwSetErrorCallback(z_engine__error_callback);
    
    if (!glfwInit()) {
        z_io__error_context("GLFW", "(%s) failed to initialize",
                            glfwGetVersionString());

        return false;
    } else {
        printf(" Using GLFW %s\n\n", glfwGetVersionString());
    }

    if (!z_engine__create_window(settings.fullscreen, settings.title)) {
        z_io__error_context("GLFW", "(%s) failed to create window",
                            glfwGetVersionString());

        return false;
    }

    if (gl3wInit()) {
        z_io__error_context("gl3w", "failed to initialize");
        
        return false;
    }
    
    z_engine__intro_gl();
    
    if (!z_engine__can_run()) {
        z_io__error("SHIZEN is not supported on this system");

        return false;
    }
    
    if (!z_gfx__init(z_engine__build_viewport())) {
        return false;
    }
    
    _graphics_context.is_initialized = true;

    z_time_reset();
    
#ifdef SHIZ_DEBUG
    if (!z_debug__init()) {
        z_io__error("SHIZEN could not initialize a debugging state");
        
        return false;
    }
#endif
    
    return true;
}

bool
z_shutdown()
{
    if (!_graphics_context.is_initialized) {
        return false;
    }

    if (!z_gfx__kill()) {
        return false;
    }

#ifdef SHIZ_DEBUG
    if (!z_debug__kill()) {
        return false;
    }
#endif
    
    z_res__unload_all();

    GLFWwindow * const window = _graphics_context.window;
    
    glfwSetWindowCloseCallback(window, NULL);
    glfwSetWindowFocusCallback(window, NULL);
    glfwSetFramebufferSizeCallback(window, NULL);
    glfwSetKeyCallback(window, NULL);
    glfwSetErrorCallback(NULL);

    glfwTerminate();
    
    _graphics_context.is_initialized = false;
    
    return true;
}

void
z_engine__present_frame()
{
    glfwSwapBuffers(_graphics_context.window);
    glfwPollEvents();
}

void
z_request_finish()
{
    _graphics_context.should_finish = true;
}

bool
z_should_finish()
{
    return _graphics_context.should_finish;
}

SHIZSize
z_get_display_size()
{
    return _graphics_context.native_size;
}

static
SHIZViewport
z_engine__build_viewport()
{
    SHIZViewport viewport = SHIZViewportDefault;

    viewport.resolution = _graphics_context.native_size;
    viewport.framebuffer = z_engine__get_framebuffer_size();
    viewport.scale = z_engine__get_pixel_scale();

    return viewport;
}

static
void
z_engine__intro(char const * const description)
{
    char const * mode = "";
    
#ifdef SHIZ_DEBUG
    mode = "DEBUG";
#else
    mode = "RELEASE";
#endif

    printf("\n");
    printf("  __|  |  | _ _| __  /  __|   \\ |\n");
    printf("\\__ \\  __ |   |     /   _|   .  |\n");
    printf("____/ _| _| ___| ____| ___| _|\\_|\n\n");
    printf(" SHIZEN %d.%d.%d / %s %s (built %s, %s)\n",
           SHIZEN_VERSION_MAJOR, SHIZEN_VERSION_MINOR, SHIZEN_VERSION_PATCH,
           SHIZEN_VERSION_NAME, mode, __DATE__, __TIME__);
    printf(" Copyright (c) 2017 Jacob Hauberg Hansen\n\n");

    if (description != NULL) {
        printf("%s\n", description);
    }

    printf(" ----------------\n");
}

static
void
z_engine__intro_gl()
{
    printf(" OPENGL VERSION:  %s (GLSL %s)\n",
           glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf(" OPENGL RENDERER: %s\n",
           glGetString(GL_RENDERER));
    printf(" OPENGL VENDOR:   %s\n\n",
           glGetString(GL_VENDOR));
}

static
bool
z_engine__can_run()
{
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

static
bool
z_engine__create_window(bool const fullscreen,
                        char const * const title)
{
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SHIZ_MIN_OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SHIZ_MIN_OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    
    SHIZSize const visible_size = _graphics_context.display_size;
    
    if (fullscreen) {
        GLFWmonitor * const monitor = glfwGetPrimaryMonitor();
        
        if (monitor) {
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            
            int const display_width = mode->width;
            int const display_height = mode->height;
            
            _graphics_context.window = glfwCreateWindow(display_width, display_height,
                                                        title, glfwGetPrimaryMonitor(), NULL);
            
            // prefer centered window if initially fullscreen;
            // otherwise let the OS determine window placement
            _preferred_window_position.x = (display_width / 2) - (visible_size.width / 2);
            _preferred_window_position.y = (display_height / 2) - (visible_size.height / 2);
        }
    } else {
        _graphics_context.window = glfwCreateWindow((int)visible_size.width,
                                                    (int)visible_size.height,
                                                    title, NULL, NULL);
    }
    
    if (!_graphics_context.window) {
        return false;
    }
    
    GLFWwindow * const window = _graphics_context.window;
    
    _graphics_context.is_fullscreen = z_engine__is_fullscreen();
    
    glfwSetWindowCloseCallback(window, z_engine__window_close_callback);
    glfwSetWindowFocusCallback(window, z_engine__window_focus_callback);
    
    glfwSetFramebufferSizeCallback(window, z_engine__framebuffer_size_callback);
    
    glfwSetKeyCallback(window, z_engine__key_callback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(_graphics_context.swap_interval);
    
    return true;
}

static
SHIZSize
z_engine__get_window_size()
{
    int window_width;
    int window_height;
    
    glfwGetWindowSize(_graphics_context.window,
                      &window_width, &window_height);
    
    return SHIZSizeMake(window_width, window_height);
}

static
SHIZSize
z_engine__get_framebuffer_size()
{
    int framebuffer_width;
    int framebuffer_height;
    
    // determine pixel size of the framebuffer for the window
    // this size is not necesarilly equal to the size of the window, as some
    // platforms may increase the pixel count (e.g. doubling on retina screens)
    glfwGetFramebufferSize(_graphics_context.window,
                           &framebuffer_width, &framebuffer_height);
    
    return SHIZSizeMake(framebuffer_width, framebuffer_height);
}

static
f32
z_engine__get_pixel_scale()
{
    SHIZSize const framebuffer = z_engine__get_framebuffer_size();
    SHIZSize const window = z_engine__get_window_size();
    
    return (framebuffer.width + framebuffer.height) /
        (window.width + window.height);
}

static
bool
z_engine__is_fullscreen()
{
    if (_graphics_context.window != NULL) {
        return glfwGetWindowMonitor(_graphics_context.window) != NULL;
    }
    
    return false;
}

static
void
z_engine__toggle_windowed(GLFWwindow * const window)
{
    bool const is_currently_fullscreen = z_engine__is_fullscreen();

    int window_position_x = (int)_preferred_window_position.x;
    int window_position_y = (int)_preferred_window_position.y;
    
    if (is_currently_fullscreen) {
        // go windowed
        glfwSetWindowMonitor(window, NULL,
                             window_position_x, window_position_y,
                             (int)_graphics_context.display_size.width,
                             (int)_graphics_context.display_size.height,
                             0);
    } else {
        // go fullscreen
        GLFWmonitor * const monitor = glfwGetPrimaryMonitor();

        if (monitor) {
            GLFWvidmode const * const mode = glfwGetVideoMode(monitor);

            glfwGetWindowPos(window,
                             &window_position_x,
                             &window_position_y);
         
            _preferred_window_position.x = window_position_x;
            _preferred_window_position.y = window_position_y;
            
            glfwSetWindowMonitor(window, monitor, 0, 0,
                                 mode->width, mode->height,
                                 mode->refreshRate);
        }
    }
    
    _graphics_context.is_fullscreen = z_engine__is_fullscreen();
}

static
void
z_engine__key_callback(GLFWwindow * const window,
                       int const key,
                       int const scancode,
                       int const action,
                       int const mods)
{
    (void)scancode;
    // todo: we should use input to handle this stuff; i.e. create specialized input key that trigger this stuff
    if ((mods == GLFW_MOD_ALT && key == GLFW_KEY_ENTER) && action == GLFW_RELEASE) {
        z_engine__toggle_windowed(window);
    }
#ifdef SHIZ_DEBUG
    else if ((key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        z_debug__toggle_enabled();
    }
    
    if (z_debug__is_enabled()) {
        z_debug__toggle_expanded(mods == GLFW_MOD_SHIFT);
        z_debug__set_is_printing_sprite_order((mods == GLFW_MOD_SHIFT &&
                                               key == GLFW_KEY_Z) &&
                                              action == GLFW_PRESS);
        
        if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_1) && action == GLFW_RELEASE) {
            z_debug__toggle_draw_shapes();
        } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_2) && action == GLFW_RELEASE) {
            z_debug__toggle_draw_events();
        } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_MINUS) && (action == GLFW_PRESS ||
                                                                         action == GLFW_REPEAT)) {
            z_time_set_scale(z_time_get_scale() - 0.1);
        } else if ((mods == GLFW_MOD_SHIFT && key == GLFW_KEY_EQUAL) && (action == GLFW_PRESS ||
                                                                         action == GLFW_REPEAT)) {
            z_time_set_scale(z_time_get_scale() + 0.1);
        }
    }
#endif
}

static
void
z_engine__error_callback(int const error,
                         char const * const description)
{
    z_io__error_context("GLFW", "%d %s", error, description);
}

static
void
z_engine__window_close_callback(GLFWwindow * const window)
{
    (void)window;

    _graphics_context.should_finish = true;
}

static
void
z_engine__window_focus_callback(GLFWwindow * const window,
                                int const focused)
{
    (void)window;

    _graphics_context.is_focused = focused;
}

static
void
z_engine__framebuffer_size_callback(GLFWwindow * const window,
                                    int const width,
                                    int const height)
{
    (void)window;
    (void)width;
    (void)height;

    _graphics_context.is_fullscreen = z_engine__is_fullscreen();
    
    SHIZViewport const viewport = z_engine__build_viewport();

    z_viewport__set(viewport);
}
