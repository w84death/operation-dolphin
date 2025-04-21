#ifndef ITEM_TYPES_H
#define ITEM_TYPES_H

#include <stdbool.h>

// Item interaction types
typedef enum {
    ITEM_INTERACTION_OPEN,  // Item can be opened (like boxes)
    ITEM_INTERACTION_TAKE   // Item can be picked up and removed from level
} ItemInteractionType;

// Item types (for categorization)
typedef enum {
    ITEM_CATEGORY_CONTAINER,  // Container that can be opened
    ITEM_CATEGORY_TOOL,       // Tools that can be taken
    ITEM_CATEGORY_WEAPON,     // Weapons that can be taken
    ITEM_CATEGORY_KEY         // Key items for progression
} ItemCategory;

// Structure to define item properties
typedef struct {
    const char* name;                   // Item name
    const char* texture_closed_path;    // Path to closed/default texture
    const char* texture_open_path;      // Path to opened texture (NULL if not applicable)
    float width;                        // Width in meters
    float height;                       // Height in meters
    ItemCategory category;              // Item category
    ItemInteractionType interaction;    // How the player interacts with this item
    bool rare;                          // Whether this is a rare item (affects spawn rate)
} ItemDefinition;

// List of all item types in the game
static const ItemDefinition ITEM_DEFINITIONS[] = {
    // name,      texture_closed_path,                 texture_open_path,                  width, height, category,             interaction,          rare
    {"Crate",     "textures/items/box_closed.tga",     "textures/items/box_open.tga",      1.0f,  1.0f,   ITEM_CATEGORY_CONTAINER, ITEM_INTERACTION_OPEN,  false},
    {"Ammo Box",  "textures/items/ammo_closed.tga",    "textures/items/ammo_open.tga",     0.5f,  0.5f,   ITEM_CATEGORY_CONTAINER, ITEM_INTERACTION_OPEN,  false},
    {"Radio",     "textures/items/radio.tga",          NULL,                               0.5f,  0.5f,   ITEM_CATEGORY_TOOL,      ITEM_INTERACTION_TAKE,  true}
};

// Number of defined item types
#define ITEM_DEFINITIONS_COUNT (sizeof(ITEM_DEFINITIONS) / sizeof(ITEM_DEFINITIONS[0]))

#endif // ITEM_TYPES_H