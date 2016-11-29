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

#ifndef internal_h
#define internal_h

#define GLFW_INCLUDE_NONE

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

typedef struct {
    bool is_initialized;
    bool is_focused;

    bool should_finish;
    
    GLFWwindow *window;
} SHIZGraphicsContext;

#endif // internal_h
