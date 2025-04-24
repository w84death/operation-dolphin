#ifndef STATIC_ELEMENTS_H
#define STATIC_ELEMENTS_H

#include <stdbool.h>
#include <GL/gl.h>
#include "config.h"
#include "item_types.h" // Include for item definitions

// Maximum number of item types that can spawn around a static element
#define MAX_SPAWNABLE_ITEMS 5

// Structure for defining what items can spawn around a static element
typedef struct {
    int item_definition_index;  // Index into ITEM_DEFINITIONS array
    int min_count;              // Minimum number of this item to spawn
    int max_count;              // Maximum number of this item to spawn
    float min_distance;         // Minimum distance from static element
    float max_distance;         // Maximum distance from static element
} SpawnableItem;

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
    bool items_spawned;   // Whether items have already been spawned around this element
};

// Structure to define static element type properties
typedef struct {
    const char* name;         // Element name
    const char* folder_path;  // Full path to folder containing sprites
    float width;              // Base width in meters
    float height;             // Base height in meters
    int num_spawnable_items;  // Number of different item types that can spawn around this element
    SpawnableItem spawnable_items[MAX_SPAWNABLE_ITEMS]; // Items that can spawn around this element
} StaticElementType;

// List of all static element types
static const StaticElementType STATIC_ELEMENT_TYPES[] = {
    // name,         folder_path,                   width, height, num_spawnable_items, spawnable_items
    {"Hut",         "textures/infrastructure/hut",  12.0f, 12.0f, 3, {
        // item_definition_index, min_count, max_count, min_distance, max_distance
        {0, 1, 3, 3.0f, 8.0f},  // Crate boxes
        {1, 1, 2, 2.0f, 6.0f},  // Ammo boxes
        {2, 0, 1, 1.0f, 4.0f},  // Radio (rare)
    }},
    {"Truck",       "textures/mobile/truck",        6.0f, 6.0f, 2, {
        // item_definition_index, min_count, max_count, min_distance, max_distance
        {1, 1, 3, 1.5f, 4.0f},  // Ammo boxes
        {2, 0, 1, 0.5f, 2.0f},  // Radio (rare)
    }}
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

// New function to spawn items around static elements
void spawnItemsAroundStaticElements(void);

// Add functions to access static element data from other modules
StaticElement* getStaticElementsArray(void);
int getStaticElementCount(void);

#endif // STATIC_ELEMENTS_H