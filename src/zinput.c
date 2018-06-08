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

#include <SHIZEN/zinput.h> // SHIZInput, z_input_*

#include "internal.h" // SHIZGraphicsContext, GLFWwindow

extern SHIZGraphicsContext const _graphics_context;

static bool _down[SHIZInputMax] = { false };
static bool _pressed[SHIZInputMax] = { false };
static bool _released[SHIZInputMax] = { false };

void
z_input_update()
{
    bool down_previously[SHIZInputMax];

    for (SHIZInput input = SHIZInputUp; input < SHIZInputMax; input++) {
        down_previously[input] = _down[input];
    }

    GLFWwindow * const window = _graphics_context.window;

    bool is_modified =
        glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;

    _down[SHIZInputUp] =
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;

    _down[SHIZInputDown] =
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

    _down[SHIZInputLeft] =
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

    _down[SHIZInputRight] =
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    _down[SHIZInputConfirm] =
        (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS) && !is_modified;

    _down[SHIZInputEscape] =
        glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !is_modified;
    
    _down[SHIZInputSpace] =
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !is_modified;
    
    _down[SHIZInputX] = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
    _down[SHIZInputZ] = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;

    _down[SHIZInputAny] = false;

    for (SHIZInput input = SHIZInputUp; input < SHIZInputMax; input++) {
        if (_down[input]) {
            _down[SHIZInputAny] = true;
        }

        _released[input] = down_previously[input] && !_down[input];
        _pressed[input] = !down_previously[input] && _down[input];
    }
}

bool
z_input_down(SHIZInput const input)
{
    return _down[input];
}

bool
z_input_pressed(SHIZInput const input)
{
    return _pressed[input];
}

bool
z_input_released(SHIZInput const input)
{
    return _released[input];
}
