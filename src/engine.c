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

#include <SHIZEN/engine.h>

#include "internal.h"

void _shiz_glfw_error_callback(int error, const char* description);

void _shiz_glfw_window_close_callback(GLFWwindow* window);
void _shiz_glfw_window_focus_callback(GLFWwindow* window, int focused);

SHIZGraphicsContext context;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

bool shiz_init() {
    if (context.is_initialized) {
        return true;
    }

    glfwSetErrorCallback(_shiz_glfw_error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "glfw: failed to initialize\n");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    const char* const title = "SHIZEN";
    
    bool const fullscreen = false;

    if (fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        context.window = glfwCreateWindow(mode->width, mode->height, title, glfwGetPrimaryMonitor(), NULL);
    } else {
        context.window = glfwCreateWindow(320, 240, title, NULL, NULL);
    }

    if (!context.window) {
        fprintf(stderr, "glfw: failed to make window\n");
        return false;
    }

    glfwSetWindowCloseCallback(context.window, _shiz_glfw_window_close_callback);
    glfwSetWindowFocusCallback(context.window, _shiz_glfw_window_focus_callback);
    
    glfwSetKeyCallback(context.window, key_callback);

    glfwMakeContextCurrent(context.window);
    glfwSwapInterval(1);

    if (gl3wInit()) {
        fprintf(stderr, "gl3w: failed to initialize\n");
        return false;
    }

    context.is_initialized = true;
    
    return true;
}

bool shiz_shutdown() {
    if (!context.is_initialized) {
        return false;
    }
    
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

void shiz_draw_begin() {
    // not yet implemented
}

void shiz_draw_end() {
    glfwSwapBuffers(context.window);
    glfwPollEvents();
}

void _shiz_glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "glfw: %d %s\n", error, description);
}

void _shiz_glfw_window_close_callback(GLFWwindow* window) {
    (void)window;

    context.should_finish = true;
}

void _shiz_glfw_window_focus_callback(GLFWwindow* window, int focused) {
    (void)window;

    context.is_focused = focused;
}
