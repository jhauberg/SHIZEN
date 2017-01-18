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
#include <stdarg.h>

#include "io.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wunused-function"
#endif

#include <stb_image.h>

#if defined(__clang__)
 #pragma clang pop
#endif

static bool _shiz_io_handle_image(unsigned char * const image_data, int width, int height, int components, shiz_io_image_loaded_handler handler);
static void _shiz_io_printf(const char * const format, va_list args);

static uint const buffer_capacity = 256;

static char buffer[buffer_capacity];
static char buffer_format[buffer_capacity];

static const char *error_context = "ERROR";
static const char *warning_context = "WARNING";

static void _shiz_io_printf(const char * const format, va_list args) {
    vsnprintf(buffer, buffer_capacity, format, args);
    fprintf(stderr, "%s\n", buffer);
}

#define SHIZ_IO_PRINTF(stmt) \
    va_list args; \
    va_start(args, format); { \
        stmt; \
        _shiz_io_printf(buffer_format, args); \
    } \
    va_end(args); \

void shiz_io_error(const char * const format, ...) {
    SHIZ_IO_PRINTF( sprintf(buffer_format, "[%s] %s", error_context, format) )
}

void shiz_io_error_context(const char * const context, const char * const format, ...) {
    SHIZ_IO_PRINTF( sprintf(buffer_format, "[%s] [%s] %s", error_context, context, format) )
}

void shiz_io_warning(const char * const format, ...) {
    SHIZ_IO_PRINTF( sprintf(buffer_format, "[%s] %s", warning_context, format) )
}

void shiz_io_warning_context(const char * const context, const char * const format, ...) {
    SHIZ_IO_PRINTF( sprintf(buffer_format, "[%s] [%s] %s", warning_context, context, format) )
}

static bool _shiz_io_handle_image(unsigned char * const image_data, int width, int height, int components, shiz_io_image_loaded_handler const handler) {
    if (handler) {
        if (!(*handler)(width, height, components, image_data)) {
            stbi_image_free(image_data);

            return false;
        }
    }

    stbi_image_free(image_data);
    
    return true;
}

bool shiz_io_load_image(const char * const filename, shiz_io_image_loaded_handler const handler) {
    int width, height;
    int components;

    // stbi defaults to reading the first pixel at the top-left of the image, however,
    // opengl expects the first pixel to be at the bottom-left of the image, so we need to flip it
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char * image = stbi_load(filename, &width, &height, &components, STBI_rgb_alpha);

    if (!image) {
        shiz_io_error("failed to load image: '%s'", filename);

        return false;
    }

    return _shiz_io_handle_image(image, width, height, components, handler);
}

bool shiz_io_load_image_data(const unsigned char * const buffer, uint const length, shiz_io_image_loaded_handler const handler) {
    int width, height;
    int components;

    stbi_set_flip_vertically_on_load(true);

    unsigned char * image = stbi_load_from_memory(buffer, length, &width, &height, &components, STBI_rgb_alpha);

    if (!image) {
        shiz_io_error("failed to load image (from memory)");
        
        return false;
    }
    
    return _shiz_io_handle_image(image, width, height, components, handler);
}
