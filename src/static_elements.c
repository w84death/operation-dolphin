#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include "../include/static_elements.h"
#include "../include/terrain.h"
#include "../include/log.h"
#include "../include/game.h"
#include "../include/items.h"  // Add include for items functions

// Include stb_image.h after all other includes
#define STBI_ONLY_TGA
#include "../stb_image.h"

// Global pointer to current game state
static void* game_state_ptr = NULL;

// Global static elements data
static StaticElement* static_elements = NULL;
static int static_element_count = 0;
static int static_element_capacity = 0;

// Textures for static element sprites (for each type and each direction)
static GLuint static_element_textures[MAX_STATIC_ELEMENT_TYPES][STATIC_ELEMENT_DIRECTIONS];
static bool static_element_textures_loaded[MAX_STATIC_ELEMENT_TYPES] = {false};

// Set the game state pointer for settings access
void setStaticElementGameStatePointer(void* game_ptr) {
    game_state_ptr = game_ptr;
}

// Load textures for all static element types
bool loadStaticElementTextures() {
    // Helper function to load a texture
    GLuint loadTexture(const char* filename) {
        // Create an OpenGL texture ID
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        
        // Bind this texture to its ID
        glBindTexture(GL_TEXTURE_2D, texture_id);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Load the image
        int width, height, channels;
        unsigned char* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
        
        // Check if loading was successful
        if (!image) {
            logError("Could not load texture: %s", filename);
            glDeleteTextures(1, &texture_id);
            return 0;
        }
        
        // Upload the image data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        
        // Free the image data as it's now in GPU memory
        stbi_image_free(image);
        
        logInfo("Loaded static element texture: %s", filename);
        return texture_id;
    }

    // Load textures for each static element type and direction
    int successful_types = 0;
    
    for (int type_idx = 0; type_idx < STATIC_ELEMENT_TYPE_COUNT && type_idx < MAX_STATIC_ELEMENT_TYPES; type_idx++) {
        const StaticElementType* element_type = &STATIC_ELEMENT_TYPES[type_idx];
        bool all_loaded = true;
        
        for (int dir = 0; dir < STATIC_ELEMENT_DIRECTIONS; dir++) {
            // Build the filename using the folder path and direction number (1-8)
            char filename[256];
            snprintf(filename, sizeof(filename), "%s/%d.tga", 
                    element_type->folder_path, dir + 1);
            
            static_element_textures[type_idx][dir] = loadTexture(filename);
            
            if (static_element_textures[type_idx][dir] == 0) {
                logWarning("Failed to load texture for static element type %s, direction %d: %s", 
                          element_type->name, dir + 1, filename);
                all_loaded = false;
            }
        }
        
        static_element_textures_loaded[type_idx] = all_loaded;
        
        if (all_loaded) {
            logInfo("Successfully loaded all textures for static element type: %s", element_type->name);
            successful_types++;
        } else {
            logWarning("Not all textures were loaded for static element type: %s", element_type->name);
        }
    }

    // Return true if at least one static element type was fully loaded
    return successful_types > 0;
}

// Function to ensure we have enough capacity for static elements
static void ensureStaticElementCapacity(int required_capacity) {
    if (static_element_capacity >= required_capacity) {
        return; // We already have enough capacity
    }
    
    // Calculate new capacity (double current or minimum required)
    int new_capacity = static_element_capacity * 2;
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }
    
    // Cap maximum capacity
    if (new_capacity > MAX_STATIC_ELEMENTS) {
        new_capacity = MAX_STATIC_ELEMENTS;
    }
    
    // Allocate or reallocate memory
    if (static_elements == NULL) {
        static_elements = (StaticElement*)malloc(new_capacity * sizeof(StaticElement));
    } else {
        StaticElement* new_elements = (StaticElement*)realloc(static_elements, new_capacity * sizeof(StaticElement));
        if (new_elements != NULL) {
            static_elements = new_elements;
        }
    }
    
    // Check if allocation was successful
    if (static_elements == NULL) {
        logError("Failed to allocate memory for static elements. Required: %d elements", required_capacity);
        static_element_capacity = 0;
        return;
    }
    
    // Update capacity
    static_element_capacity = new_capacity;
    logInfo("Increased static element capacity to %d", static_element_capacity);
}

// Create static elements on the terrain
void createStaticElements(int count, float terrain_size) {
    // If count is zero, do nothing
    if (count <= 0) return;
    
    // Use game state for seed if available, otherwise use timestamp
    unsigned int seed = time(NULL);
    GameState* game = (GameState*)game_state_ptr;
    
    if (game != NULL) {
        seed = game->settings.foliage_seed;
        logInfo("Using seed for static element placement: %u", seed);
    }
    
    // Set random seed
    srand(seed);
    
    // Make sure we have enough capacity
    ensureStaticElementCapacity(count);
    
    // Calculate world position
    float half_size = terrain_size / 2.0f;
    float terrain_offset_x = TERRAIN_POSITION_X;
    float terrain_offset_z = TERRAIN_POSITION_Z;
    float ground_level = TERRAIN_POSITION_Y;
    
    // Count how many static element types have their textures loaded
    int available_types_count = 0;
    for (int i = 0; i < STATIC_ELEMENT_TYPE_COUNT && i < MAX_STATIC_ELEMENT_TYPES; i++) {
        if (static_element_textures_loaded[i]) {
            available_types_count++;
        }
    }
    
    // Return if no static element types are available
    if (available_types_count == 0) {
        logError("No static element types available with loaded textures");
        return;
    }
    
    // Clear existing static elements
    static_element_count = 0;
    
    // Create static elements
    for (int i = 0; i < count && i < static_element_capacity; i++) {
        // Select a random type from the ones with loaded textures
        int type_offset = rand() % available_types_count;
        int type_index = -1;
        
        // Find the nth loaded type
        for (int j = 0, found = 0; j < STATIC_ELEMENT_TYPE_COUNT; j++) {
            if (static_element_textures_loaded[j]) {
                if (found == type_offset) {
                    type_index = j;
                    break;
                }
                found++;
            }
        }
        
        // Skip this element if we couldn't find a valid type
        if (type_index < 0) continue;
        
        // Store the type index
        static_elements[i].type_index = type_index;
        const StaticElementType* element_type = &STATIC_ELEMENT_TYPES[type_index];
        
        // Place elements randomly on the terrain
        static_elements[i].x = ((float)rand() / RAND_MAX) * terrain_size - half_size + terrain_offset_x;
        static_elements[i].z = ((float)rand() / RAND_MAX) * terrain_size - half_size + terrain_offset_z;
        
        // Fixed rotation for static element (random rotation for variety)
        static_elements[i].rotation = ((float)rand() / RAND_MAX) * 360.0f;
        
        // Set dimensions based on element type with some small random variation (+/- 10%)
        float scale_variation = 0.9f + ((float)rand() / RAND_MAX) * 0.2f;  // 0.9 to 1.1
        static_elements[i].width = element_type->width * scale_variation;
        static_elements[i].height = element_type->height * scale_variation;
        
        // Position on ground level with small offset to avoid z-fighting
        static_elements[i].y = ground_level + 0.05f;
        
        // Chunk coordinates (for future use with chunks)
        static_elements[i].chunk_x = 0;
        static_elements[i].chunk_z = 0;
        
        // Activate the static element
        static_elements[i].active = true;
        
        static_element_count++;
        
        logInfo("Created static element %d: type=%s, position=(%.2f, %.2f, %.2f), size=%.2fx%.2f", 
                i, element_type->name, static_elements[i].x, static_elements[i].y, static_elements[i].z, 
                static_elements[i].width, static_elements[i].height);
    }
    
    logInfo("Created %d static elements on the terrain", static_element_count);
}

// Create static elements for a specific chunk
void createStaticElementsForChunk(int chunk_x, int chunk_z, float chunk_size, unsigned int seed) {
    // Not implemented yet - will be similar to vegetation chunk system
    // This will be useful for procedural terrain generation
}

// Draw a billboard that always faces the camera
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture) {
    // Only draw if we have a valid texture
    if (texture == 0) return;

    // Save the current matrix
    glPushMatrix();
    
    // Position at the base of the billboard
    glTranslatef(x, y, z);
    
    // Get the current modelview matrix
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    // Create a modelview matrix for the billboard that only contains position
    modelview[0] = 1.0f;
    modelview[1] = 0.0f;
    modelview[2] = 0.0f;
    
    modelview[4] = 0.0f;
    modelview[5] = 1.0f;
    modelview[6] = 0.0f;
    
    modelview[8] = 0.0f;
    modelview[9] = 0.0f;
    modelview[10] = 1.0f;
    
    glLoadMatrixf(modelview);
    
    // Enable texturing and proper alpha blending
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use alpha testing to avoid rendering transparent pixels
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f); // Lower alpha threshold to 0.1 for better visibility
    
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Half width for quad vertices
    float half_width = width / 2.0f;
    
    // Use a slightly brighter color to make static elements stand out
    glColor4f(1.2f, 1.2f, 1.0f, 1.0f);
    
    // Draw quad
    glBegin(GL_QUADS);
    
    // Bottom left - anchored at ground level
    glTexCoord2f(0.0f, 1.0f);  // Flip texture coordinates vertically
    glVertex3f(-half_width, 0.0f, 0.0f);
    
    // Bottom right
    glTexCoord2f(1.0f, 1.0f);  // Flip texture coordinates vertically
    glVertex3f(half_width, 0.0f, 0.0f);
    
    // Top right
    glTexCoord2f(1.0f, 0.0f);  // Flip texture coordinates vertically
    glVertex3f(half_width, height, 0.0f);
    
    // Top left
    glTexCoord2f(0.0f, 0.0f);  // Flip texture coordinates vertically
    glVertex3f(-half_width, height, 0.0f);
    
    glEnd();
    
    // Restore state
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    
    // Restore the previous matrix
    glPopMatrix();
}

// Render all static elements with directional sprites
void renderStaticElements(float camera_x, float camera_z) {
    // For properly lit static elements, we need to keep lighting enabled
    // but disable lighting's influence on color values temporarily
    GLfloat ambient[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    // Save current material state
    GLfloat orig_ambient[4], orig_diffuse[4];
    glGetMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glGetMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
    
    // Set material to fully accept light without changing the color
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    
    // Make sure textures properly interact with lighting
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Render each static element with the appropriate directional sprite
    for (int i = 0; i < static_element_count; i++) {
        if (static_elements[i].active) {
            // Skip if type index is invalid
            if (static_elements[i].type_index < 0 || static_elements[i].type_index >= STATIC_ELEMENT_TYPE_COUNT) {
                continue;
            }
            
            // Calculate angle between camera and static element in the XZ plane
            float dx = camera_x - static_elements[i].x;
            float dz = camera_z - static_elements[i].z;
            
            // Calculate viewing angle in degrees (0-360)
            float view_angle = atan2f(dx, dz) * 180.0f / M_PI;
            if (view_angle < 0) view_angle += 360.0f;
            
            // Calculate the relative angle between the camera view direction and the static element's rotation
            // This determines which of the 8 directional textures to show
            float relative_angle = view_angle - static_elements[i].rotation;
            
            // Normalize to 0-360 range
            while (relative_angle < 0) relative_angle += 360.0f;
            while (relative_angle >= 360.0f) relative_angle -= 360.0f;
            
            // Calculate which of the 8 direction sprites to use (45-degree segments)
            // Split the 360 degrees into 8 segments of 45 degrees each
            int direction_index = (int)((relative_angle + 22.5f) / 45.0f) % STATIC_ELEMENT_DIRECTIONS;
            
            // Reverse the direction to match texture ordering
            direction_index = (STATIC_ELEMENT_DIRECTIONS - 1) - direction_index;
            
            // Get the texture for this static element type and direction
            GLuint texture = static_element_textures[static_elements[i].type_index][direction_index];
            
            // Draw the static element sprite - using fixed billboard to always face camera
            drawBillboard(
                static_elements[i].x, 
                static_elements[i].y, 
                static_elements[i].z, 
                static_elements[i].width, 
                static_elements[i].height, 
                texture
            );
        }
    }
    
    // Restore original material state
    glMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
}

// Clean up static element resources
void cleanupStaticElements(void) {
    if (static_elements != NULL) {
        free(static_elements);
        static_elements = NULL;
    }
    static_element_count = 0;
    static_element_capacity = 0;
}

// Spawn items around static elements based on their definitions
void spawnItemsAroundStaticElements(void) {
    // Get access to the terrain for height checks
    GameState* game = (GameState*)game_state_ptr;
    Terrain* terrain = game ? (Terrain*)game->terrain : NULL;
    
    if (terrain == NULL) {
        log_error("Cannot spawn items around static elements: terrain is NULL");
        return;
    }
    
    log_info("Spawning items around static elements...");
    
    // Use the same seed as the game for consistent spawning
    unsigned int seed = game->settings.foliage_seed;
    srand(seed);
    
    int total_items_spawned = 0;
    
    // Process each active static element
    for (int i = 0; i < static_element_count; i++) {
        if (!static_elements[i].active || static_elements[i].items_spawned) {
            continue; // Skip inactive elements or those that already have items
        }
        
        int type_index = static_elements[i].type_index;
        if (type_index < 0 || type_index >= STATIC_ELEMENT_TYPE_COUNT) {
            continue; // Skip invalid type indices
        }
        
        const StaticElementType* element_type = &STATIC_ELEMENT_TYPES[type_index];
        int element_items_spawned = 0;
        
        // Process each item type that can spawn around this element
        for (int j = 0; j < element_type->num_spawnable_items && j < MAX_SPAWNABLE_ITEMS; j++) {
            const SpawnableItem* spawnable_item = &element_type->spawnable_items[j];
            
            // Check if the item definition index is valid
            if (spawnable_item->item_definition_index >= ITEM_DEFINITIONS_COUNT) {
                log_warning("Invalid item definition index %d for static element type %s",
                           spawnable_item->item_definition_index, element_type->name);
                continue;
            }
            
            // Determine how many of this item to spawn, applying the spawn multiplier
            int min_count = spawnable_item->min_count;
            int max_count = (int)(spawnable_item->max_count * ITEM_SPAWN_MULTIPLIER);
            
            // Ensure we have at least the minimum count
            if (max_count < min_count) {
                max_count = min_count;
            }
            
            int count_to_spawn;
            if (min_count == max_count) {
                count_to_spawn = min_count;
            } else {
                count_to_spawn = min_count + (rand() % (max_count - min_count + 1));
            }
            
            // Spawn the specified number of items
            for (int k = 0; k < count_to_spawn; k++) {
                // Generate a random angle and distance for item placement
                float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;  // 0 to 2π
                float distance_range = spawnable_item->max_distance - spawnable_item->min_distance;
                float distance = spawnable_item->min_distance + ((float)rand() / RAND_MAX) * distance_range;
                
                // Calculate position relative to the static element
                float offset_x = cosf(angle) * distance;
                float offset_z = sinf(angle) * distance;
                
                float item_x = static_elements[i].x + offset_x;
                float item_z = static_elements[i].z + offset_z;
                
                // Get the height at this position from the terrain
                float item_y = getHeightAtPoint(terrain, item_x, item_z);
                
                // Add a small offset to ensure the item sits on top of the terrain
                float item_height = ITEM_DEFINITIONS[spawnable_item->item_definition_index].height;
                item_y += item_height * 0.5f;
                
                // Create the item at this position
                createSpecificItem(spawnable_item->item_definition_index, item_x, item_y, item_z);
                element_items_spawned++;
                total_items_spawned++;
            }
        }
        
        // Mark this static element as having its items spawned
        static_elements[i].items_spawned = true;
        
        log_info("Spawned %d items around %s at [%.2f, %.2f, %.2f]",
               element_items_spawned, element_type->name,
               static_elements[i].x, static_elements[i].y, static_elements[i].z);
    }
    
    log_success("Spawned a total of %d items around static elements", total_items_spawned);
}