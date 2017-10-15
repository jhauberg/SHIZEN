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

#include "io.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#if defined(__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wunused-function"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wpadded"
#endif

#include <stb/stb_image.h>

#if defined(__clang__)
 #pragma clang diagnostic pop
#endif

#include <stb/stb_vorbis.h>

#include <stdio.h> // fprintf, sprintf, vsnprintf
#include <stdarg.h> // va_list etc.

static
bool
z_io__handle_image(unsigned char * image_data,
                   int width, int height, int components,
                   z_io__load_image_handler handler);

static
void
z_io__printf(char const * format, va_list args);


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
    SHIZ_IO_PRINTF( sprintf(_buffer_format, "[%s] %s",
                            SHIZIOContextError, format) )
}

void
z_io__error_context(char const * const context,
                    char const * const format, ...)
{
    SHIZ_IO_PRINTF( sprintf(_buffer_format, "[%s] [%s] %s",
                            SHIZIOContextError, context, format) )
}

void
z_io__warning(char const * const format, ...)
{
    SHIZ_IO_PRINTF( sprintf(_buffer_format, "[%s] %s",
                            SHIZIOContextWarning, format) )
}

void
z_io__warning_context(char const * const context,
                      char const * const format, ...)
{
    SHIZ_IO_PRINTF( sprintf(_buffer_format, "[%s] [%s] %s",
                            SHIZIOContextWarning, context, format) )
}

#ifdef SHIZ_DEBUG
void
z_io__debug(char const * const format, ...)
{
    SHIZ_IO_PRINTF( sprintf(_buffer_format, "[%s] %s",
                            SHIZIOContextDebug, format) )
}
#endif

bool
z_io__load_image(char const * const filename,
                 z_io__load_image_handler const handler)
{
    int width, height;
    int components;

    // stbi defaults to reading the first pixel at the top-left of the image,
    // however, opengl expects the first pixel to be at the bottom-left of
    // the image, so we need to flip it
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char * const image =
        stbi_load(filename, &width, &height, &components,
                  STBI_rgb_alpha);

    if (!image) {
        z_io__error("failed to load image: '%s'", filename);

        return false;
    }

    return z_io__handle_image(image, width, height, components, handler);
}

bool
z_io__load_image_data(unsigned char const * const buffer,
                      unsigned int const length,
                      z_io__load_image_handler const handler)
{
    int width, height;
    int components;

    stbi_set_flip_vertically_on_load(true);

    unsigned char * const image =
        stbi_load_from_memory(buffer, (int)length, &width, &height, &components,
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
    int channels;
    int sample_rate;
    
    short * data = NULL;
    
    int size = stb_vorbis_decode_filename(filename, &channels, &sample_rate, &data);
    
    if (size == -1 || size == -2) {
        if (data != NULL) {
            free(data);
        }
        
        return false;
    }
    
    int length = size * channels * (int)sizeof(short);
    
    if (handler) {
        return (*handler)(channels, sample_rate, data, length);
    }
    
    free(data);
    
    return true;
}

static
bool
z_io__handle_image(unsigned char * const image_data,
                   int const width, int const height, int const components,
                   z_io__load_image_handler const handler)
{
    if (handler) {
        if (!(*handler)(width, height, components, image_data)) {
            stbi_image_free(image_data);
            
            return false;
        }
    }
    
    stbi_image_free(image_data);
    
    return true;
}

static
void
z_io__printf(char const * const format,
             va_list args)
{
    vsnprintf(_buffer, SHIZIOBufferCapacity, format, args);
    
    fprintf(stderr, "%s\n", _buffer);
}
