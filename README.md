# SHIZEN

A low-profile and cross-platform 2D game engine that leaves a small footprint, providing shape, sprite and text-rendering.

It is implemented *purely* in **C99** and requires **OpenGL Core Profile 3.3**.

**This is very much a work-in-progress. Beware.**

## Features

* **Sprite batching as the default.** Always render sprites in batches to reduce the number of draw calls.
* **Primitive shape drawing.** Render common shapes: e.g. rectangles, circles, paths and points.
* **Text drawing from bitmap fonts.** Render fixed-width text from a spritesheet/bitmap font. Word-wrapping and truncation is automatically handled.
* **Layering.** Make sure things are rendered in the order you expect by specifying layers.

## Dependencies

A few dependencies are required to keep the scope of the project down:

* [`GLFW`](https://github.com/glfw/glfw) handles **cross-platform window creation**
* [`gl3w`](https://github.com/skaslev/gl3w) handles **OpenGL Core Profile** header loading
* [`linmath`](https://github.com/datenwolf/linmath.h) provides **math functions**
* [`stb_image`](https://github.com/nothings/stb) provides **image loading** capabilities

## Motivation

I've always wanted to make my own videogame. I've never fully succeeded at it, and possibly never will.

This, however, is the first step.

...A step i've taken several times before, *ahem*. But let's not get into that. It's all about the journey anyway, right?

## Inspiration

* [`whitgl`](https://github.com/whitingjp/whitgl) by Jonathan Whiting
* [`Punity`](https://github.com/martincohen/Punity) by Martin Cohen
