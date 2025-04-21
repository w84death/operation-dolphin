#ifndef ITEMS_H
#define ITEMS_H

#include <stdbool.h>
#include <GL/gl.h>
#include "player.h"
#include "terrain.h"
#include "item_types.h"  // Include the new item types header

// Item instance structure (represents an item placed in the game world)
typedef struct {
    float x, y, z;             // Position in the game world
    bool active;               // Whether this item is active
    bool opened;               // Whether the item has been opened (for openable items)
    bool taken;                // Whether the item has been taken (for takeable items)
    int definition_index;      // Index into the ITEM_DEFINITIONS array
    GLuint texture_closed;     // Texture when closed/untaken (cached from the definition)
    GLuint texture_open;       // Texture when opened (if applicable, cached from the definition)
} Item;

// Function declarations
bool initItems(void);
void createItems(int count, float terrain_size, Terrain* terrain);
void renderItems(void);
void checkItemCollisions(Player* player);
void cleanupItems(void);

// New functions to support the array-based system
void createSpecificItem(int definition_index, float x, float y, float z);
const char* getItemName(int definition_index);
ItemInteractionType getItemInteraction(int definition_index);
ItemCategory getItemCategory(int definition_index);

#endif // ITEMS_H