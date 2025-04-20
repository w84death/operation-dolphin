#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include "../include/vegetation.h"
#include "../include/terrain.h"
#include "../include/log.h"
#include "../include/particles.h"
#include "../include/game.h"

// Global pointer to current game state
static void* game_state_ptr = NULL;

// Global vegetation data
static Vegetation* vegetation = NULL;
static int vegetation_count = 0;
static int vegetation_capacity = 0;

// Textures for vegetation billboards
static GLuint vegetation_textures_small[MAX_TEXTURES_PER_SIZE];
static GLuint vegetation_textures_medium[MAX_TEXTURES_PER_SIZE];
static GLuint vegetation_textures_big[MAX_TEXTURES_PER_SIZE];
static int small_texture_count = 0;
static int medium_texture_count = 0;
static int big_texture_count = 0;

// Set the game state pointer for settings access
void setGameStatePointer(void* game_ptr) {
    game_state_ptr = game_ptr;
}

// Load textures for all vegetation types
bool loadVegetationTextures() {
    // Small vegetation textures (flowers, grass)
    const char* small_textures[] = {
        "textures/foliage/small/grass1.tga",
        "textures/foliage/small/grass2.tga",
        "textures/foliage/small/grass3.tga",
        "textures/foliage/small/grass4.tga",
        "textures/foliage/small/grass5.tga",
        "textures/foliage/small/rock1.tga",
        "textures/foliage/small/rock2.tga",
        "textures/foliage/small/rock3.tga",
        "textures/foliage/small/rock4.tga",
        "textures/foliage/small/mushroom1.tga",
        "textures/foliage/small/mushroom2.tga"
    };
    
    // Medium vegetation textures (bushes, aloe, small rocks)
    const char* medium_textures[] = {
        "textures/foliage/medium/aloes1.tga",
        "textures/foliage/medium/aloes2.tga",
        "textures/foliage/medium/aloes3.tga",
        "textures/foliage/medium/aloes4.tga",
        "textures/foliage/medium/flower1.tga",
        "textures/foliage/medium/flower2.tga",
        "textures/foliage/medium/flower3.tga",
        "textures/foliage/medium/flower4.tga",
        "textures/foliage/medium/flower5.tga",
        "textures/foliage/medium/flower6.tga",
        "textures/foliage/medium/flower7.tga",
        "textures/foliage/medium/flower8.tga",
        "textures/foliage/medium/flower9.tga",
        "textures/foliage/medium/flower10.tga",
        "textures/foliage/medium/flower11.tga",
        "textures/foliage/medium/flower12.tga",
        "textures/foliage/medium/bush1.tga",
        "textures/foliage/medium/bush2.tga",
        "textures/foliage/medium/bush3.tga",
        "textures/foliage/medium/bush4.tga",
        "textures/foliage/medium/weed1.tga",
        "textures/foliage/medium/weed2.tga",
        "textures/foliage/medium/weed3.tga"    
    };
    
    // Big vegetation textures (trees, palms, big rocks)
    const char* big_textures[] = {
        "textures/foliage/big/palm1.tga",
        "textures/foliage/big/palm2.tga",
        "textures/foliage/big/palm3.tga",
        "textures/foliage/big/palm4.tga",
        "textures/foliage/big/tree1.tga",
        "textures/foliage/big/tree2.tga",
        "textures/foliage/big/tree3.tga",
        "textures/foliage/big/tree4.tga",
        "textures/foliage/big/rock2.tga",
        "textures/foliage/big/rock4.tga",
        "textures/foliage/big/oldtree1.tga",
        "textures/foliage/big/oldtree2.tga",
        "textures/foliage/big/oldtree3.tga",
        "textures/foliage/big/oldtree4.tga",
        "textures/foliage/big/oldtree5.tga"
    };
    
    // Load small textures
    small_texture_count = 0;
    for (int i = 0; i < sizeof(small_textures) / sizeof(small_textures[0]); i++) {
        if (small_texture_count >= MAX_TEXTURES_PER_SIZE) break;
        
        vegetation_textures_small[small_texture_count] = loadTexture(small_textures[i]);
        if (vegetation_textures_small[small_texture_count] == 0) {
            logWarning("Warning: Failed to load small vegetation texture: %s\n", small_textures[i]);
        } else {
            logInfo("Loaded small vegetation texture %d: %s\n", small_texture_count, small_textures[i]);
            small_texture_count++;
        }
    }
    
    // Load medium textures
    medium_texture_count = 0;
    for (int i = 0; i < sizeof(medium_textures) / sizeof(medium_textures[0]); i++) {
        if (medium_texture_count >= MAX_TEXTURES_PER_SIZE) break;
        
        vegetation_textures_medium[medium_texture_count] = loadTexture(medium_textures[i]);
        if (vegetation_textures_medium[medium_texture_count] == 0) {
            logWarning("Warning: Failed to load medium vegetation texture: %s\n", medium_textures[i]);
        } else {
            logInfo("Loaded medium vegetation texture %d: %s\n", medium_texture_count, medium_textures[i]);
            medium_texture_count++;
        }
    }
    
    // Load big textures
    big_texture_count = 0;
    for (int i = 0; i < sizeof(big_textures) / sizeof(big_textures[0]); i++) {
        if (big_texture_count >= MAX_TEXTURES_PER_SIZE) break;
        
        vegetation_textures_big[big_texture_count] = loadTexture(big_textures[i]);
        if (vegetation_textures_big[big_texture_count] == 0) {
            logWarning("Warning: Failed to load big vegetation texture: %s\n", big_textures[i]);
        } else {
            logInfo("Loaded big vegetation texture %d: %s\n", big_texture_count, big_textures[i]);
            big_texture_count++;
        }
    }
    
    if (small_texture_count == 0 || medium_texture_count == 0 || big_texture_count == 0) {
        logError("Error: Failed to load any vegetation textures in at least one category.\n");
        return false;
    }
    
    return true;
}

// Create random vegetation with three size categories (legacy function for backward compatibility)
void createVegetation(int count, float terrain_size) {
    // Free previous vegetation if any
    cleanupVegetation();
    
    // Get the game state to access the foliage seed
    GameState* game = (GameState*)game_state_ptr;
    unsigned int seed = FOLIAGE_DEFAULT_SEED; // Default seed as fallback
    
    // Use the seed from game settings if available
    if (game != NULL) {
        seed = game->settings.foliage_seed;
        logInfo("Using foliage seed from settings: %u", seed);
    }
    
    // Set the global terrain seed to match our foliage seed
    setGlobalTerrainSeed(seed);
    
    // Create vegetation for a single chunk (0,0) using the configured seed
    createVegetationForChunk(0, 0, terrain_size, seed);
}

// Clean up vegetation resources
void cleanupVegetation(void) {
    if (vegetation != NULL) {
        free(vegetation);
        vegetation = NULL;
    }
    vegetation_count = 0;
    vegetation_capacity = 0;
}

// Function to ensure we have enough capacity for vegetation
static void ensureVegetationCapacity(int required_capacity) {
    if (vegetation_capacity >= required_capacity) {
        return; // We already have enough capacity
    }
    
    // Calculate new capacity (double current or minimum required)
    int new_capacity = vegetation_capacity == 0 ? 
                       required_capacity : 
                       vegetation_capacity * 2;
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }
    
    // Reallocate the vegetation array
    Vegetation* new_vegetation = realloc(vegetation, sizeof(Vegetation) * new_capacity);
    if (new_vegetation == NULL) {
        logError("Failed to reallocate memory for vegetation (requested capacity: %d)\n", new_capacity);
        return; // Failed to allocate
    }
    
    vegetation = new_vegetation;
    vegetation_capacity = new_capacity;
    
    logInfo("Resized vegetation array to capacity: %d\n", vegetation_capacity);
}

// Create vegetation for a specific chunk with a specific seed
void createVegetationForChunk(int chunk_x, int chunk_z, float chunk_size, unsigned int seed) {
    
    // Apply density multipliers to calculate actual counts for each category
    int count_small = (int)(TERRAIN_MAX_FEATURES * VEGETATION_DENSITY_SMALL);
    int count_medium = (int)(TERRAIN_MAX_FEATURES * VEGETATION_DENSITY_MEDIUM);
    int count_big = (int)(TERRAIN_MAX_FEATURES * VEGETATION_DENSITY_BIG);
    
    // Get quality setting from game state to adjust foliage density
    GameState* game = (GameState*)game_state_ptr;
    if (game && !game->settings.high_terrain_features) {
        count_small = count_small / 2;
        count_medium = count_medium / 2;
        count_big = count_big / 2;
        logInfo("Low quality mode: reducing vegetation density by 50%%\n");
    }
    
    // Calculate total vegetation needed
    int total_count = count_small + count_medium + count_big;
    
    // Make sure we have enough capacity
    int required_capacity = vegetation_count + total_count;
    ensureVegetationCapacity(required_capacity);
    
    // Initialize random seed for this chunk
    unsigned int chunk_seed = seed + (unsigned int)((chunk_x * 73856093) ^ (chunk_z * 19349663));
    srand(chunk_seed);
    
    logInfo("Creating vegetation for chunk (%d,%d) with seed %u: %d small, %d medium, %d big\n", 
           chunk_x, chunk_z, chunk_seed, count_small, count_medium, count_big);
    
    // Calculate world position for this chunk
    float half_size = chunk_size / 2.0f;
    float chunk_offset_x = chunk_x * chunk_size;
    float chunk_offset_z = chunk_z * chunk_size;
    int current_index = vegetation_count;
    
    // Ground level from terrain settings
    float ground_level = TERRAIN_POSITION_Y;
    
    // Create small vegetation (flowers, grass)
    for (int i = 0; i < count_small; i++) {
        if (current_index >= vegetation_capacity) break;
        
        // Get a scale factor for this vegetation instance (0.8-1.2 range)
        float scale_factor = 0.8f + ((float)rand() / RAND_MAX) * 0.4f;
        
        vegetation[current_index].x = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_x;
        vegetation[current_index].z = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_z;
        vegetation[current_index].type = 0;  // Small type
        vegetation[current_index].texture_index = rand() % small_texture_count;
        
        // Apply random scaling to base dimensions
        float base_width = 0.5f + ((float)rand() / RAND_MAX) * 0.5f;
        float base_height = 0.3f + ((float)rand() / RAND_MAX) * 0.3f;
        vegetation[current_index].width = base_width * scale_factor;
        vegetation[current_index].height = base_height * scale_factor;
        
        // Place vegetation ON the ground surface (not buried)
        vegetation[current_index].y = ground_level + 0.01f; // Slightly above ground level to prevent z-fighting
        
        // Store which chunk this vegetation belongs to
        vegetation[current_index].chunk_x = chunk_x;
        vegetation[current_index].chunk_z = chunk_z;
        vegetation[current_index].active = true;
        current_index++;
    }
    
    // Create medium vegetation (bushes, aloes)
    for (int i = 0; i < count_medium; i++) {
        if (current_index >= vegetation_capacity) break;
        
        // Get a scale factor for this vegetation instance (0.7-1.3 range for medium vegetation)
        float scale_factor = 0.7f + ((float)rand() / RAND_MAX) * 0.6f;
        
        vegetation[current_index].x = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_x;
        vegetation[current_index].z = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_z;
        vegetation[current_index].type = 1;  // Medium type
        vegetation[current_index].texture_index = rand() % medium_texture_count;
        
        // Apply random scaling to base dimensions
        float base_width = 1.5f + ((float)rand() / RAND_MAX) * 1.0f;
        float base_height = 1.5f + ((float)rand() / RAND_MAX) * 1.5f;
        vegetation[current_index].width = base_width * scale_factor;
        vegetation[current_index].height = base_height * scale_factor;
        
        // Place medium vegetation ON the ground surface (not buried)
        vegetation[current_index].y = ground_level + 0.01f; // Slightly above ground level
        
        // Store which chunk this vegetation belongs to
        vegetation[current_index].chunk_x = chunk_x;
        vegetation[current_index].chunk_z = chunk_z;
        vegetation[current_index].active = true;
        current_index++;
    }
    
    // Create big vegetation (trees, palms)
    for (int i = 0; i < count_big; i++) {
        if (current_index >= vegetation_capacity) break;
        
        // Get a scale factor for this vegetation instance (0.6-1.4 range for big vegetation)
        float scale_factor = 0.6f + ((float)rand() / RAND_MAX) * 0.8f;
        
        vegetation[current_index].x = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_x;
        vegetation[current_index].z = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_z;
        vegetation[current_index].type = 2;  // Big type
        vegetation[current_index].texture_index = rand() % big_texture_count;
        
        // Apply random scaling to base dimensions
        float base_width = 3.0f + ((float)rand() / RAND_MAX) * 2.0f;
        float base_height = 5.0f + ((float)rand() / RAND_MAX) * 3.0f;
        vegetation[current_index].width = base_width * scale_factor;
        vegetation[current_index].height = base_height * scale_factor;
        
        // Place vegetation properly on the ground (y position)
        vegetation[current_index].y = ground_level + 0.01f; // Slightly above ground level
        
        // Store which chunk this vegetation belongs to
        vegetation[current_index].chunk_x = chunk_x;
        vegetation[current_index].chunk_z = chunk_z;
        vegetation[current_index].active = true;
        current_index++;
    }
    
    // Update the total vegetation count
    vegetation_count = current_index;
    
    // Restore the global random seed
    srand(time(NULL));
}

// Draw a billboard that always faces the camera
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture) {
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
    
    // Enable texturing and proper alpha blending for vegetation
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Critical fix: use alpha testing to avoid transparent pixels being drawn at all
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f); // Only render pixels with alpha > 0.5
    
    // glDepthMask(GL_FALSE); // Don't write to depth buffer for transparent objects
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Half width and height for quad vertices
    float half_width = width / 2.0f;
    
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
    glDepthMask(GL_TRUE); // Re-enable depth writing
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    
    // Restore the previous matrix
    glPopMatrix();
}

// Render all vegetation
void renderVegetation() {
    // For properly lit vegetation, we need to keep lighting enabled
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

    // Sort vegetation by type and distance before rendering
    // This ensures proper transparency layering (back-to-front)
    
    // First render big vegetation (trees, palms) since they're typically furthest
    for (int i = 0; i < vegetation_count; i++) {
        if (vegetation[i].active && vegetation[i].type == 2) {
            GLuint texture = vegetation_textures_big[vegetation[i].texture_index];
            drawBillboard(
                vegetation[i].x, 
                vegetation[i].y, 
                vegetation[i].z, 
                vegetation[i].width, 
                vegetation[i].height, 
                texture
            );
        }
    }
    
    // Then render medium vegetation (bushes, aloe)
    for (int i = 0; i < vegetation_count; i++) {
        if (vegetation[i].active && vegetation[i].type == 1) {
            GLuint texture = vegetation_textures_medium[vegetation[i].texture_index];
            drawBillboard(
                vegetation[i].x, 
                vegetation[i].y, 
                vegetation[i].z, 
                vegetation[i].width, 
                vegetation[i].height, 
                texture
            );
        }
    }
    
    // Finally render small vegetation (grass, flowers)
    for (int i = 0; i < vegetation_count; i++) {
        if (vegetation[i].active && vegetation[i].type == 0) {
            GLuint texture = vegetation_textures_small[vegetation[i].texture_index];
            drawBillboard(
                vegetation[i].x, 
                vegetation[i].y, 
                vegetation[i].z, 
                vegetation[i].width, 
                vegetation[i].height, 
                texture
            );
        }
    }
    
    // Restore original material state
    glMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
}

// Cut medium-sized foliage in front of the player
void cutMediumFoliage(Player* player) {
    if (!player->is_cutting) {
        return; // Not in cutting animation
    }
    
    // Coordinates for the player's direction vector
    float dirX = sinf(player->yaw * M_PI / 180.0f);
    float dirZ = -cosf(player->yaw * M_PI / 180.0f); // Negative for OpenGL coordinate system
    
    // Check each vegetation entity
    for (int i = 0; i < vegetation_count; i++) {
        // Only consider medium vegetation (type 1) that's active
        if (vegetation[i].type == 1 && vegetation[i].active) {
            // Calculate vector from player to vegetation
            float vecX = vegetation[i].x - player->position_x;
            float vecZ = vegetation[i].z - player->position_z;
            
            // Calculate distance (squared) in the XZ plane
            float distSquared = vecX * vecX + vecZ * vecZ;
            
            // Check if within cutting range
            if (distSquared <= FOLIAGE_CUTTING_RANGE * FOLIAGE_CUTTING_RANGE) {
                // Calculate dot product to determine if vegetation is in front of the player
                float dotProduct = vecX * dirX + vecZ * dirZ;
                
                // Vegetation is in front if dot product is positive
                if (dotProduct > 0) {
                    // Get the texture ID for this vegetation
                    GLuint texture = vegetation_textures_medium[vegetation[i].texture_index];
                    
                    // Spawn particle effect using the vegetation's texture
                    spawnFoliageParticles(vegetation[i].x, vegetation[i].y + vegetation[i].height * 0.5f, 
                                         vegetation[i].z, texture);
                    
                    // Deactivate the vegetation (cut it)
                    vegetation[i].active = false;
                    logInfo("Cut medium foliage at position (%f, %f, %f) in chunk (%d,%d)\n", 
                           vegetation[i].x, vegetation[i].y, vegetation[i].z,
                           vegetation[i].chunk_x, vegetation[i].chunk_z);
                }
            }
        }
    }
}