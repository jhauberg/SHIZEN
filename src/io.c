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

#include "io.h" // z_io_*

#include <stdint.h> // uint8_t, int16_t, uint32_t, int32_t
#include <stdio.h> // fprintf, sprintf, vsnprintf
#include <stdarg.h> // va_list

#include <stb/stb_vorbis.h> // stb_vorbis_*

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wunused-function"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wpadded"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#include <stb/stb_image.h> // stbi_*

#if defined(__clang__)
 #pragma clang diagnostic pop
#endif

static bool z_io__handle_image(uint8_t * data, int32_t width, int32_t height, int32_t components, z_io__load_image_handler);
static void z_io__printf(char const * format, va_list args);

#define SHIZIOBufferCapacity 256

static char _buffer[SHIZIOBufferCapacity];
static char _buffer_format[SHIZIOBufferCapacity];

#define SHIZIOContextError   "!ERROR!"
#define SHIZIOContextWarning "WARNING"

#ifdef SHIZ_DEBUG
 #define SHIZIOContextDebug  "*DEBUG*"
#endif

#define SHIZ_IO_PRINTF(stmt) \
    va_list args; \
    va_start(args, format); { \
        stmt; \
        z_io__printf(_buffer_format, args); \
    } \
    va_end(args); \

void
z_io__error(char const * const format, ...)
{
    SHIZ_IO_PRINTF(sprintf(_buffer_format, "[%s] %s",
                           SHIZIOContextError, format))
}

void
z_io__error_context(char const * const context,
                    char const * const format,
                    ...)
{
    SHIZ_IO_PRINTF(sprintf(_buffer_format, "[%s] [%s] %s",
                           SHIZIOContextError, context, format))
}

void
z_io__warning(char const * const format, ...)
{
    SHIZ_IO_PRINTF(sprintf(_buffer_format, "[%s] %s",
                           SHIZIOContextWarning, format))
}

void
z_io__warning_context(char const * const context,
                      char const * const format, ...)
{
    SHIZ_IO_PRINTF(sprintf(_buffer_format, "[%s] [%s] %s",
                           SHIZIOContextWarning, context, format))
}

#ifdef SHIZ_DEBUG
void
z_io__debug(char const * const format, ...)
{
    SHIZ_IO_PRINTF(sprintf(_buffer_format, "[%s] %s",
                           SHIZIOContextDebug, format))
}
#endif

bool
z_io__load_image(char const * const filename,
                 z_io__load_image_handler const handler)
{
    int32_t width, height;
    int32_t components;

    // stbi defaults to reading the first pixel at the top-left of the image,
    // however, opengl expects the first pixel to be at the bottom-left of
    // the image, so we need to flip it
    stbi_set_flip_vertically_on_load(true);
    
    uint8_t * const image =
        stbi_load(filename, &width, &height, &components,
                  STBI_rgb_alpha);

    if (!image) {
        z_io__error("failed to load image: '%s'", filename);

        return false;
    }

    return z_io__handle_image(image, width, height, components, handler);
}

bool
z_io__load_image_data(uint8_t const * const buffer,
                      uint32_t const length,
                      z_io__load_image_handler const handler)
{
    int32_t width, height;
    int32_t components;

    stbi_set_flip_vertically_on_load(true);

    unsigned char * const image =
        stbi_load_from_memory(buffer, (int32_t)length, &width, &height, &components,
                              STBI_rgb_alpha);

    if (!image) {
        z_io__error("failed to load image (from memory)");
        
        return false;
    }
    
    return z_io__handle_image(image, width, height, components, handler);
}

bool
z_io__load_sound(char const * const filename,
                 z_io__load_sound_handler const handler)
{
    int32_t channels;
    int32_t sample_rate;
    
    int16_t * data = NULL;
    
    int size = stb_vorbis_decode_filename(filename, &channels, &sample_rate, &data);
    
    if (size == -1 || size == -2) {
        if (data != NULL) {
            free(data);
        }
        
        return false;
    }
    
    int32_t length = size * channels * (int32_t)sizeof(uint16_t);
    
    if (handler) {
        return (*handler)(channels, sample_rate, data, length);
    }
    
    free(data);
    
    return true;
}

static
bool
z_io__handle_image(uint8_t * const data,
                   int32_t const width,
                   int32_t const height,
                   int32_t const components,
                   z_io__load_image_handler const handler)
{
    if (handler) {
        if (!(*handler)(width, height, components, data)) {
            stbi_image_free(data);
            
            return false;
        }
    }
    
    stbi_image_free(data);
    
    return true;
}

static
void
z_io__printf(char const * const format, va_list args)
{
    vsnprintf(_buffer, SHIZIOBufferCapacity, format, args);
    
    fprintf(stderr, "%s\n", _buffer);
}
