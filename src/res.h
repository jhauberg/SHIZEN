// A resource system for loading and unloading sounds and images.
//
// Images, once loaded, are immediately uploaded to the GPU in an OpenGL
// texture buffer, and are kept there until unloaded.
//
// Similarly, sounds are kept in memory, in an OpenAL buffer, until unloaded.
//
// All resources are accessed, used and manipulated through id's.

#pragma once

#include <stdbool.h> // bool
#include <stdint.h> // uint8_t, uint16_t

typedef enum SHIZResourceType {
    SHIZResourceTypeNotSupported,
    SHIZResourceTypeImage,
    SHIZResourceTypeSound
} SHIZResourceType;

typedef struct SHIZResourceImage {
    char const * filename;
    uint32_t texture_id;
    uint16_t width;
    uint16_t height;
    uint8_t resource_id;
} SHIZResourceImage;

typedef struct SHIZResourceSound {
    char const * filename;
    uint32_t source_id;
    uint32_t buffer_id;
    uint8_t resource_id;
} SHIZResourceSound;

extern SHIZResourceImage const SHIZResourceImageEmpty;
extern SHIZResourceSound const SHIZResourceSoundEmpty;

extern uint8_t const SHIZResourceInvalid;

uint8_t z_res__load(char const * filename);
uint8_t z_res__load_data(SHIZResourceType, uint8_t const * buffer, uint32_t length);

bool z_res__unload(uint8_t resource_id);
bool z_res__unload_all(void);

SHIZResourceType const z_res__type(char const * filename);

SHIZResourceImage z_res__image(uint8_t resource_id);
SHIZResourceSound z_res__sound(uint8_t resource_id);
