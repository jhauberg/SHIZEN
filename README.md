# SHIZEN

A low-profile and cross-platform 2D game engine<sup>\*</sup> that leaves a small footprint, providing shape, sprite and text-rendering using modern and efficient techniques.

It is written *entirely* in **C99** and requires **OpenGL 3.3 (Core Profile)**.

**This is very much a work-in-progress. Beware.**

## Features

* **Sprite batching as the default.** Sprites are always rendered in efficient batches to reduce the number of draw calls.
* **Text drawing from bitmap fonts.** Supports text rendering using bitmap fonts. Word-wrapping and truncation is automatically handled.
* **Smooth and stutter-free rendering.** Animate values smoothly under any frame-rate by blending between frames.
* **Primitive shape drawing.** Supports rendering common shapes: e.g. rectangles, circles, paths and points.
* **Layering.** Sprites, text and primitives are always rendered in the expected order by specifying layers.

<sub>\* Calling it an engine is probably going too far. It's more like a graphics framework that facilitates game development.</sub>

## Dependencies

A few dependencies are required to keep the scope of the project down. Most of these are small and already included as part of the repository (see [external](/external)).

**Requires installation:**

* [`GLFW`](https://github.com/glfw/glfw) handles **cross-platform window creation**

**Included:**

* [`gl3w`](https://github.com/skaslev/gl3w) handles **OpenGL Core Profile** header loading
* [`linmath`](https://github.com/datenwolf/linmath.h) provides **math functions**
* [`stb_image`](https://github.com/nothings/stb) provides **image loading** capabilities
* [`PCG`](http://www.pcg-random.org) for improved **random number generation**

## Examples

Take a look in [examples](/examples).

## Motivation

I've always wanted to make my own videogame. I've never (fully) succeeded at it, and possibly never will.

This, however, is the first step... A step i've taken several times before, *ahem*.

But let's not get into that. It's all about the journey anyway, right?

## Inspiration

### Similar projects

* [`whitgl`](https://github.com/whitingjp/whitgl) by Jonathan Whiting
* [`Punity`](https://github.com/martincohen/Punity) by Martin Cohen
* [`Allegro`](https://github.com/liballeg/allegro5) by Community/Shawn Hargreaves

### Articles

["Order your graphics draw calls around!"](http://realtimecollisiondetection.net/blog/?p=86)

This short blog post by Christer Ericson has been very helpful and inspirational to me in the past, and also provided the idea behind the implementation of how sprites are sorted and ordered in SHIZEN.

["The Lost Art of C Structure Packing"](http://www.catb.org/esr/structure-packing/#_readability_and_cache_locality)

Interesting and valuable knowledge regarding structure layout in regards to performance/memory usage.

In this day and age, this is not stuff that we typically (have to) think much about, but could actually have a significant positive impact if considered.

["My favorite C programming practices"](https://github.com/mcinglis/c-style)

I'm a sucker for following rules/best practices and strive to keep my code consistent and readable.

This article has helped a lot. Though I've decided on other practices in some cases, as a general guideline, this one is great.

["Fix Your Timestep!"](http://gafferongames.com/game-physics/fix-your-timestep/)

Essential reading for implementing frame-rate independent simulation.
