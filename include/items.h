#ifndef ITEMS_H
#define ITEMS_H

#include <stdbool.h>
#include <GL/gl.h>
#include "player.h"
#include "terrain.h"

// Item types
typedef enum {
    ITEM_TYPE_BOX,     // Crate box
    ITEM_TYPE_AMMO     // Ammo box
} ItemType;

// Item structure
typedef struct {
    float x, y, z;      // Position
    float width, height; // Dimensions
    bool active;         // Whether this item is active
    bool opened;         // Whether the item has been opened
    ItemType type;       // Type of item
    GLuint texture_closed; // Texture when closed
    GLuint texture_open;   // Texture when opened
} Item;

// Function declarations
bool initItems(void);
void createItems(int count, float terrain_size, Terrain* terrain);
void renderItems(void);
void checkItemCollisions(Player* player);
void cleanupItems(void);

#endif // ITEMS_H