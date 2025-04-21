#ifndef STATIC_ELEMENTS_H
#define STATIC_ELEMENTS_H

#include <stdbool.h>
#include <GL/gl.h>
#include "config.h"

// Forward declaration of StaticElement
typedef struct StaticElement StaticElement;

// Type definition for StaticElement
struct StaticElement {
    float x, y, z;       // Position
    float width, height; // Dimensions
    float rotation;      // Rotation in degrees (0-360, fixed for static elements)
    int type_index;      // Index to determine which texture set to use
    bool active;         // Whether this element is visible
    int chunk_x, chunk_z; // Which chunk this static element belongs to
};

// Structure to define static element type properties
typedef struct {
    const char* name;         // Element name
    const char* folder_path;  // Full path to folder containing sprites
    float width;              // Base width in meters
    float height;             // Base height in meters
} StaticElementType;

// List of all static element types
static const StaticElementType STATIC_ELEMENT_TYPES[] = {
    // name,         folder_path,                   width, height
    {"Hut",         "textures/infrastructure/hut",  12.0f,  12.0f},
    {"Truck",       "textures/mobile/truck",        6.0f,  6.0f}
};

// Number of static element types
#define STATIC_ELEMENT_TYPE_COUNT (sizeof(STATIC_ELEMENT_TYPES) / sizeof(STATIC_ELEMENT_TYPES[0]))

// Static element related functions
void setStaticElementGameStatePointer(void* game_ptr);
bool loadStaticElementTextures(void);
void createStaticElements(int count, float terrain_size);
void createStaticElementsForChunk(int chunk_x, int chunk_z, float chunk_size, unsigned int seed);
void renderStaticElements(float camera_x, float camera_z);
void cleanupStaticElements(void);

#endif // STATIC_ELEMENTS_H