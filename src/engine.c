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

static void _shiz_intro(void);
static bool _shiz_can_run(void);
#ifdef SHIZ_DEBUG
static void _shiz_process_errors(void);
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

bool shiz_init() {
    if (context.is_initialized) {
        return true;
    }

    glfwSetErrorCallback(_shiz_glfw_error_callback);
    
    if (!glfwInit()) {
        shiz_io_error("GLFW", "(%s) failed to initialize", glfwGetVersionString());

        return false;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SHIZ_MIN_OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SHIZ_MIN_OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    const char* const title = "SHIZEN";
    
    bool const fullscreen = false;

    if (fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        context.window = glfwCreateWindow(mode->width, mode->height,
                                          title, glfwGetPrimaryMonitor(), NULL);
    } else {
        context.window = glfwCreateWindow(320, 240, title, NULL, NULL);
    }

    if (!context.window) {
        shiz_io_error("GLFW", "(%s) failed to create window", glfwGetVersionString());

        return false;
    }
    
    glfwSetWindowCloseCallback(context.window, _shiz_glfw_window_close_callback);
    glfwSetWindowFocusCallback(context.window, _shiz_glfw_window_focus_callback);
    
    glfwSetKeyCallback(context.window, key_callback);

    glfwMakeContextCurrent(context.window);
    glfwSwapInterval(1);

    if (gl3wInit()) {
        shiz_io_error_context("gl3w", "failed to initialize");
        
        return false;
    }
    
    if (!_shiz_can_run()) {
        shiz_io_error("SHIZEN is not supported on this system");

        return false;
    }
    
    if (!shiz_gfx_init()) {
        return false;
    }
    
    _shiz_intro();

    context.is_initialized = true;

    glClearColor(0, 0, 0, 1);

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

void shiz_drawing_begin() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shiz_drawing_end() {
#ifdef SHIZ_DEBUG
    _shiz_process_errors();
#endif
    glfwSwapBuffers(context.window);
    glfwPollEvents();
}

void shiz_draw_line(SHIZPoint const from, SHIZPoint const to, SHIZColor const color) {
    SHIZPoint points[] = {
        from, to
    };

    shiz_draw_path(points, 2, color);
}

void shiz_draw_path(SHIZPoint const points[], uint const count, SHIZColor const color) {
    SHIZVertexPositionColor vertices[count];

    for (uint i = 0; i < count; i++) {
        vertices[i].position = points[i];
        vertices[i].color = color;
    }

    shiz_gfx_render(GL_LINE_STRIP, vertices, count);
}

void shiz_draw_rect(SHIZRect const rect, SHIZColor const color) {
    uint const vertex_count = 4;

    SHIZVertexPositionColor vertices[vertex_count];

    for (uint i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
    }

    float const l = rect.center.x - (rect.width / 2.0f);
    float const r = rect.center.x + (rect.width / 2.0f);
    float const b = rect.center.y - (rect.height / 2.0f);
    float const t = rect.center.y + (rect.height / 2.0f);

    vertices[0].position = SHIZPointMake(l, b);
    vertices[1].position = SHIZPointMake(l, t);
    vertices[2].position = SHIZPointMake(r, b);
    vertices[3].position = SHIZPointMake(r, t);

    shiz_gfx_render(GL_TRIANGLE_STRIP, vertices, vertex_count);
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
static void _shiz_process_errors() {
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
