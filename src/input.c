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

#include <SHIZEN/input.h>

#include "internal_type.h"

extern SHIZGraphicsContext const _context;

static bool down[SHIZInputMax] = { false };
static bool pressed[SHIZInputMax] = { false };
static bool released[SHIZInputMax] = { false };

void
shiz_input_update()
{
    bool down_previously[SHIZInputMax];

    for (SHIZInput input = SHIZInputUp; input < SHIZInputMax; input++) {
        down_previously[input] = down[input];
    }

    GLFWwindow * const window = _context.window;

    bool is_modified =
        glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;

    down[SHIZInputUp] =
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;

    down[SHIZInputDown] =
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

    down[SHIZInputLeft] =
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

    down[SHIZInputRight] =
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    down[SHIZInputConfirm] =
        (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS) && !is_modified;

    down[SHIZInputEscape] =
        glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !is_modified;

    down[SHIZInputAny] = false;

    for (SHIZInput input = SHIZInputUp; input < SHIZInputMax; input++) {
        if (down[input]) {
            down[SHIZInputAny] = true;
        }

        released[input] = down_previously[input] && !down[input];
        pressed[input] = !down_previously[input] && down[input];
    }
}

bool
shiz_input_down(SHIZInput const input)
{
    return down[input];
}

bool
shiz_input_pressed(SHIZInput const input)
{
    return pressed[input];
}

bool
shiz_input_released(SHIZInput const input)
{
    return released[input];
}
