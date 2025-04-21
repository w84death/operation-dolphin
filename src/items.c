#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "../include/items.h"
#include "../include/config.h"
#include "../include/log.h"

// Include stb_image.h after all other includes
#define STBI_ONLY_TGA
#include "../stb_image.h" // For texture loading

// Global items array
static Item items[MAX_ITEMS];
static int item_count = 0;

// Array to store texture IDs for each item definition
static GLuint item_textures_closed[ITEM_DEFINITIONS_COUNT] = {0};
static GLuint item_textures_open[ITEM_DEFINITIONS_COUNT] = {0};

// Forward declaration for utility function
static GLuint loadItemTexture(const char* texture_path);
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture);

// Initialize the items system and load textures
bool initItems(void) {
    log_info("Initializing items system with %d item definitions...", ITEM_DEFINITIONS_COUNT);
    
    // Load textures for all item definitions
    for (int i = 0; i < ITEM_DEFINITIONS_COUNT; i++) {
        // Load closed/default texture
        item_textures_closed[i] = loadItemTexture(ITEM_DEFINITIONS[i].texture_closed_path);
        
        // Load open texture only if it exists
        if (ITEM_DEFINITIONS[i].texture_open_path != NULL) {
            item_textures_open[i] = loadItemTexture(ITEM_DEFINITIONS[i].texture_open_path);
        } else {
            item_textures_open[i] = 0; // No open texture
        }
        
        // Check if texture loading was successful
        if (item_textures_closed[i] == 0) {
            log_error("Failed to load texture for item '%s'", ITEM_DEFINITIONS[i].name);
            return false;
        }
        
        if (ITEM_DEFINITIONS[i].texture_open_path != NULL && item_textures_open[i] == 0) {
            log_error("Failed to load open texture for item '%s'", ITEM_DEFINITIONS[i].name);
            return false;
        }
    }
    
    // Initialize all items as inactive
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
        items[i].opened = false;
        items[i].taken = false;
    }
    
    log_success("Items system initialized successfully with %d item definitions", ITEM_DEFINITIONS_COUNT);
    return true;
}

// Create items randomly distributed on the terrain
void createItems(int count, float terrain_size, Terrain* terrain) {
    log_info("Creating %d items...", count);
    
    // Reset item count
    item_count = 0;
    
    // Clear any existing items
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
        items[i].opened = false;
        items[i].taken = false;
    }
    
    // Limit count to maximum
    if (count > MAX_ITEMS) {
        count = MAX_ITEMS;
        log_warning("Item count limited to %d", MAX_ITEMS);
    }
    
    // Create random items
    for (int i = 0; i < count; i++) {
        // Generate random position within terrain bounds
        float x = ((float)rand() / RAND_MAX - 0.5f) * terrain_size;
        float z = ((float)rand() / RAND_MAX - 0.5f) * terrain_size;
        
        // Get the terrain height at this position
        float y = getHeightAtPoint(terrain, x, z);
        
        // Choose a random item definition with weighting for rarity
        int def_index;
        
        // 80% chance for common items, 20% chance for rare items
        if (rand() % 100 < 80) {
            // Find a common item
            int attempts = 0;
            do {
                def_index = rand() % ITEM_DEFINITIONS_COUNT;
                attempts++;
            } while (ITEM_DEFINITIONS[def_index].rare && attempts < 10);
            
            // If we couldn't find a common item after several attempts, just use any
            if (attempts >= 10) {
                def_index = rand() % ITEM_DEFINITIONS_COUNT;
            }
        } else {
            // Find a rare item
            int attempts = 0;
            do {
                def_index = rand() % ITEM_DEFINITIONS_COUNT;
                attempts++;
            } while (!ITEM_DEFINITIONS[def_index].rare && attempts < 10);
            
            // If we couldn't find a rare item after several attempts, just use any
            if (attempts >= 10) {
                def_index = rand() % ITEM_DEFINITIONS_COUNT;
            }
        }
        
        // Add a small offset to ensure items sit on top of the terrain and don't clip through
        y += ITEM_DEFINITIONS[def_index].height * 0.5f;
        
        // Set up item instance
        items[i].x = x;
        items[i].y = y;
        items[i].z = z;
        items[i].active = true;
        items[i].opened = false;
        items[i].taken = false;
        items[i].definition_index = def_index;
        
        // Assign textures from the precached array
        items[i].texture_closed = item_textures_closed[def_index];
        items[i].texture_open = item_textures_open[def_index];
        
        item_count++;
    }
    
    log_success("Created %d items", item_count);
}

// Create a specific item at a given position
void createSpecificItem(int definition_index, float x, float y, float z) {
    // Check if the definition index is valid
    if (definition_index < 0 || definition_index >= ITEM_DEFINITIONS_COUNT) {
        log_error("Invalid item definition index: %d", definition_index);
        return;
    }
    
    // Check if we have space for more items
    if (item_count >= MAX_ITEMS) {
        log_error("Cannot create more items, already at maximum capacity (%d)", MAX_ITEMS);
        return;
    }
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot was found, return an error
    if (slot == -1) {
        log_error("No free slots found for new item, despite item_count < MAX_ITEMS");
        return;
    }
    
    // Set up the item
    items[slot].x = x;
    items[slot].y = y;
    items[slot].z = z;
    items[slot].active = true;
    items[slot].opened = false;
    items[slot].taken = false;
    items[slot].definition_index = definition_index;
    items[slot].texture_closed = item_textures_closed[definition_index];
    items[slot].texture_open = item_textures_open[definition_index];
    
    // Increment item count
    item_count++;
    
    log_info("Created item '%s' at position [%.2f, %.2f, %.2f]", 
             ITEM_DEFINITIONS[definition_index].name, x, y, z);
}

// Render all active items
void renderItems(void) {
    // Save current material properties and states
    float orig_ambient[4], orig_diffuse[4];
    glGetMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glGetMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
    
    // Material for items - brighter to make them stand out more
    float ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    // Set material to fully accept light without changing the color
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    
    // Make sure textures properly interact with lighting
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Save depth test state and temporarily disable depth writing for alpha blending
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    
    // Render all items
    for (int i = 0; i < MAX_ITEMS; i++) {
        // Only render active items that haven't been taken
        if (items[i].active && !items[i].taken) {
            int def_index = items[i].definition_index;
            
            // Choose texture based on opened state
            GLuint texture;
            if (ITEM_DEFINITIONS[def_index].interaction == ITEM_INTERACTION_OPEN) {
                texture = items[i].opened ? items[i].texture_open : items[i].texture_closed;
            } else {
                texture = items[i].texture_closed;
            }
            
            // Draw the item as a billboard
            drawBillboard(
                items[i].x, 
                items[i].y, 
                items[i].z, 
                ITEM_DEFINITIONS[def_index].width, 
                ITEM_DEFINITIONS[def_index].height, 
                texture
            );
        }
    }
    
    // Restore depth writing state
    glDepthMask(depthMask);
    
    // Restore original material state
    glMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
}

// Check for collisions between the player and items
void checkItemCollisions(Player* player) {
    // Only check for collisions if player is actively in the game
    if (player == NULL) return;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        // Skip inactive items
        if (!items[i].active) continue;
        
        // Get the item definition
        int def_index = items[i].definition_index;
        ItemInteractionType interaction = ITEM_DEFINITIONS[def_index].interaction;
        
        // Skip items that have already been interacted with
        if ((interaction == ITEM_INTERACTION_OPEN && items[i].opened) ||
            (interaction == ITEM_INTERACTION_TAKE && items[i].taken)) {
            continue;
        }
        
        // Calculate distance to player
        float dx = items[i].x - player->position_x;
        float dz = items[i].z - player->position_z;
        float distance = sqrtf(dx * dx + dz * dz);
        
        // Check if within interaction range
        if (distance < ITEM_INTERACTION_RANGE) {
            if (interaction == ITEM_INTERACTION_OPEN) {
                // Open the item
                items[i].opened = true;
                
                // Log the interaction
                log_info("Player opened %s at position [%.2f, %.2f, %.2f]", 
                        ITEM_DEFINITIONS[def_index].name,
                        items[i].x, items[i].y, items[i].z);
            } 
            else if (interaction == ITEM_INTERACTION_TAKE) {
                // Take the item
                items[i].taken = true;
                
                // Log the interaction
                log_info("Player picked up %s at position [%.2f, %.2f, %.2f]", 
                        ITEM_DEFINITIONS[def_index].name,
                        items[i].x, items[i].y, items[i].z);
            }
            
            // In future versions, this would give the player the item contents
            // For now, we're just changing the texture or removing the item
        }
    }
}

// Clean up item resources
void cleanupItems(void) {
    log_info("Cleaning up items resources...");
    
    // Delete all textures
    for (int i = 0; i < ITEM_DEFINITIONS_COUNT; i++) {
        if (item_textures_closed[i] != 0) {
            glDeleteTextures(1, &item_textures_closed[i]);
            item_textures_closed[i] = 0;
        }
        
        if (item_textures_open[i] != 0) {
            glDeleteTextures(1, &item_textures_open[i]);
            item_textures_open[i] = 0;
        }
    }
    
    // Reset all items
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
    }
    
    item_count = 0;
}

// Get the name of an item based on its definition index
const char* getItemName(int definition_index) {
    if (definition_index >= 0 && definition_index < ITEM_DEFINITIONS_COUNT) {
        return ITEM_DEFINITIONS[definition_index].name;
    }
    return "Unknown Item";
}

// Get the interaction type of an item based on its definition index
ItemInteractionType getItemInteraction(int definition_index) {
    if (definition_index >= 0 && definition_index < ITEM_DEFINITIONS_COUNT) {
        return ITEM_DEFINITIONS[definition_index].interaction;
    }
    return ITEM_INTERACTION_TAKE; // Default
}

// Get the category of an item based on its definition index
ItemCategory getItemCategory(int definition_index) {
    if (definition_index >= 0 && definition_index < ITEM_DEFINITIONS_COUNT) {
        return ITEM_DEFINITIONS[definition_index].category;
    }
    return ITEM_CATEGORY_CONTAINER; // Default
}

// Load a texture from file using stb_image
static GLuint loadItemTexture(const char* texture_path) {
    if (texture_path == NULL) {
        return 0;
    }
    
    log_info("Loading item texture: %s", texture_path);
    
    // Load image using stb_image
    int width, height, channels;
    unsigned char *image = stbi_load(texture_path, &width, &height, &channels, STBI_rgb_alpha);
    
    if (!image) {
        log_error("Failed to load texture: %s", stbi_failure_reason());
        return 0;
    }
    
    // Generate OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // Free the image data as it's now in GPU memory
    stbi_image_free(image);
    
    log_success("Texture loaded successfully, ID: %u", texture_id);
    return texture_id;
}

// Draw a billboard (quad that always faces the camera) - Adapted from vegetation.c
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture) {
    // Only draw if we have a valid texture
    if (texture == 0) return;

    // Save the current matrix
    glPushMatrix();
    
    // Position at the base of the billboard (adjust y slightly if needed, maybe y - height * 0.5f ?)
    glTranslatef(x, y, z); // Using the item's center y for now
    
    // Get the current modelview matrix
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    // Create a modelview matrix for the billboard that only contains position (removes rotation)
    // This ensures the billboard always faces the camera plane
    modelview[0] = 1.0f; modelview[1] = 0.0f; modelview[2] = 0.0f;
    modelview[4] = 0.0f; modelview[5] = 1.0f; modelview[6] = 0.0f;
    modelview[8] = 0.0f; modelview[9] = 0.0f; modelview[10] = 1.0f;
    
    // Load the modified matrix
    glLoadMatrixf(modelview);
    
    // Enable texturing and proper alpha blending/testing
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use alpha testing to discard fully transparent pixels
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f); // Adjust threshold if needed (0.1 to 0.5 is common)
    
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Half width and height for quad vertices
    float half_width = width / 2.0f;
    float half_height = height / 2.0f; // Draw centered around the y position
    
    // Draw quad centered around the origin (which is now at x, y, z)
    glBegin(GL_QUADS);
    
    // Bottom left 
    glTexCoord2f(0.0f, 1.0f); // Flipped Y-coord like vegetation? Check item textures. Assuming standard for now.
    glVertex3f(-half_width, -half_height, 0.0f); 
    
    // Bottom right
    glTexCoord2f(1.0f, 1.0f); 
    glVertex3f(half_width, -half_height, 0.0f);
    
    // Top right
    glTexCoord2f(1.0f, 0.0f); 
    glVertex3f(half_width, half_height, 0.0f);
    
    // Top left
    glTexCoord2f(0.0f, 0.0f); 
    glVertex3f(-half_width, half_height, 0.0f);
    
    glEnd();
    
    // Restore state
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    
    // Restore the previous matrix
    glPopMatrix();
}