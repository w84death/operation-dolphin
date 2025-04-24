#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include "../include/animals.h"
#include "../include/terrain.h"
#include "../include/log.h"
#include "../include/game.h"

// Include stb_image.h after all other includes
#define STBI_ONLY_TGA
#include "../stb_image.h"

// Global pointer to current game state
static void* game_state_ptr = NULL;

// Global animals data
static Animal* animals = NULL;
static int animal_count = 0;
static int animal_capacity = 0;

// Textures for animal sprites (for each species and each direction)
static GLuint animal_textures[MAX_ANIMAL_SPECIES][ANIMAL_DIRECTIONS];
static bool animal_textures_loaded[MAX_ANIMAL_SPECIES] = {false};

// Set the game state pointer for settings access
void setAnimalGameStatePointer(void* game_ptr) {
    game_state_ptr = game_ptr;
}

// Load textures for all animal types
bool loadAnimalTextures() {
    // Helper function to load a texture
    GLuint loadTexture(const char* filename) {
        // Create an OpenGL texture ID
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        
        // Set this texture to be the one we're working with
        glBindTexture(GL_TEXTURE_2D, texture_id);
        
        // Try loading the texture using stb_image (helper function)
        int width, height, channels;
        unsigned char* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
        
        if (!image) {
            logError("Failed to load texture: %s - %s", filename, stbi_failure_reason());
            return 0;
        }
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Upload the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        
        // Free the image data as it's now in GPU memory
        stbi_image_free(image);
        
        logInfo("Loaded animal texture: %s", filename);
        return texture_id;
    }

    // Load textures for each animal species and direction
    int successful_species = 0;
    
    for (int species_idx = 0; species_idx < ANIMAL_SPECIES_COUNT && species_idx < MAX_ANIMAL_SPECIES; species_idx++) {
        const AnimalSpecies* species = &ANIMAL_SPECIES[species_idx];
        bool all_loaded = true;
        
        for (int dir = 0; dir < ANIMAL_DIRECTIONS; dir++) {
            // Build the filename using the folder name and direction number (1-8)
            char filename[256];
            snprintf(filename, sizeof(filename), "textures/animals/%s/%d.tga", 
                    species->folder_name, dir + 1);
            
            animal_textures[species_idx][dir] = loadTexture(filename);
            
            if (animal_textures[species_idx][dir] == 0) {
                logWarning("Failed to load texture for animal type %s, direction %d: %s", 
                          species->name, dir + 1, filename);
                all_loaded = false;
            }
        }
        
        animal_textures_loaded[species_idx] = all_loaded;
        
        if (all_loaded) {
            logInfo("Successfully loaded all textures for animal type: %s", species->name);
            successful_species++;
        } else {
            logWarning("Not all textures were loaded for animal type: %s", species->name);
        }
    }

    // Return true if at least one animal species was fully loaded
    return successful_species > 0;
}

// Ensure the animal array has enough capacity
static void ensureAnimalCapacity(int required_capacity) {
    if (animal_capacity >= required_capacity) {
        return; // Already have enough capacity
    }
    
    // Calculate new capacity (double the current size, or at least the required capacity)
    int new_capacity = animal_capacity == 0 ? 
                       required_capacity : 
                       animal_capacity * 2;
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }
    
    // Reallocate the animals array
    Animal* new_animals = realloc(animals, sizeof(Animal) * new_capacity);
    if (new_animals == NULL) {
        logError("Failed to reallocate memory for animals (requested capacity: %d)", new_capacity);
        return; // Failed to allocate
    }
    
    animals = new_animals;
    animal_capacity = new_capacity;
    
    logInfo("Resized animal array to capacity: %d", animal_capacity);
}

// Create animals across the terrain
void createAnimals(int count, float terrain_size) {
    // Free previous animals if any
    cleanupAnimals();
    
    // Get the game state to access the seed
    GameState* game = (GameState*)game_state_ptr;
    unsigned int seed = FOLIAGE_DEFAULT_SEED; // Default seed as fallback
    
    // Use the seed from game settings if available
    if (game != NULL) {
        seed = game->settings.foliage_seed;
        logInfo("Using seed for animal placement: %u", seed);
    }
    
    // Set random seed
    srand(seed);
    
    // Calculate the number of chunks in each dimension
    int chunks_per_side = TERRAIN_TILES_COUNT;
    float chunk_size = terrain_size;
    
    // Distribute animals evenly across all chunks
    int animals_per_chunk = count / (chunks_per_side * chunks_per_side);
    int remaining_animals = count % (chunks_per_side * chunks_per_side);
    
    logInfo("Creating animals across %dx%d chunks, ~%d per chunk", 
           chunks_per_side, chunks_per_side, animals_per_chunk);
    
    // Create animals for each chunk
    for (int z = 0; z < chunks_per_side; z++) {
        for (int x = 0; x < chunks_per_side; x++) {
            // Calculate how many animals to place in this chunk
            int chunk_animal_count = animals_per_chunk;
            
            // Distribute any remaining animals to the first chunks
            if (remaining_animals > 0) {
                chunk_animal_count++;
                remaining_animals--;
            }
            
            // Create animals for this chunk
            createAnimalsForChunk(x - chunks_per_side/2, z - chunks_per_side/2, 
                               chunk_size, seed, chunk_animal_count);
        }
    }
    
    logInfo("Created a total of %d animals across %d chunks", animal_count, 
           chunks_per_side * chunks_per_side);
}

// Create animals for a specific chunk
void createAnimalsForChunk(int chunk_x, int chunk_z, float chunk_size, unsigned int seed, int count) {
    if (count <= 0) return;
    
    // Initialize random seed for this chunk
    unsigned int chunk_seed = seed + (unsigned int)((chunk_x * 73856093) ^ (chunk_z * 19349663));
    srand(chunk_seed);
    
    // Make sure we have enough capacity
    ensureAnimalCapacity(animal_count + count);
    
    // Calculate world position for this chunk
    float half_size = chunk_size / 2.0f;
    float chunk_offset_x = chunk_x * chunk_size;
    float chunk_offset_z = chunk_z * chunk_size;
    float ground_level = TERRAIN_POSITION_Y;
    
    // Count how many animal species have their textures loaded
    int available_species_count = 0;
    for (int i = 0; i < ANIMAL_SPECIES_COUNT && i < MAX_ANIMAL_SPECIES; i++) {
        if (animal_textures_loaded[i]) {
            available_species_count++;
        }
    }
    
    // Return if no animal species are available
    if (available_species_count == 0) {
        logError("No animal species available with loaded textures");
        return;
    }
    
    // Create animals for this chunk
    for (int i = 0; i < count; i++) {
        int current_index = animal_count;
        if (current_index >= animal_capacity) break;
        
        // Select a random species from the ones with loaded textures
        int species_offset = rand() % available_species_count;
        int species_index = -1;
        
        // Find the nth loaded species
        for (int j = 0, found = 0; j < ANIMAL_SPECIES_COUNT; j++) {
            if (animal_textures_loaded[j]) {
                if (found == species_offset) {
                    species_index = j;
                    break;
                }
                found++;
            }
        }
        
        // Skip this animal if we couldn't find a valid species
        if (species_index < 0) continue;
        
        // Store the species index
        animals[current_index].species_index = species_index;
        const AnimalSpecies* species = &ANIMAL_SPECIES[species_index];
        
        // Place animals randomly within this chunk
        animals[current_index].x = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_x;
        animals[current_index].z = ((float)rand() / RAND_MAX) * chunk_size - half_size + chunk_offset_z;
        
        // Set dimensions based on species
        animals[current_index].width = species->width;
        animals[current_index].height = species->height;
        
        // Random initial rotation and direction
        animals[current_index].rotation = ((float)rand() / RAND_MAX) * 360.0f;
        animals[current_index].direction = animals[current_index].rotation;
        
        // Position on ground level with small offset to avoid z-fighting
        animals[current_index].y = ground_level + 0.1f;
        
        // Set initial state and movement parameters
        animals[current_index].state = ANIMAL_IDLE;
        animals[current_index].state_timer = ANIMAL_MIN_IDLE_TIME + 
                                    ((float)rand() / RAND_MAX) * 
                                    (ANIMAL_MAX_IDLE_TIME - ANIMAL_MIN_IDLE_TIME);
        animals[current_index].velocity = 0.0f;
        animals[current_index].max_velocity = species->speed * (0.8f + ((float)rand() / RAND_MAX) * 0.4f);
        
        // Handle flying animals
        if (species->behavior == ANIMAL_FLYING) {
            animals[current_index].flight_height = FLYING_MIN_HEIGHT + 
                                         ((float)rand() / RAND_MAX) * 
                                         (FLYING_MAX_HEIGHT - FLYING_MIN_HEIGHT);
            animals[current_index].y += animals[current_index].flight_height;
            animals[current_index].vertical_velocity = 0.0f;
            animals[current_index].ascending = ((float)rand() / RAND_MAX) > 0.5f;
        }
        
        // Store chunk coordinates
        animals[current_index].chunk_x = chunk_x;
        animals[current_index].chunk_z = chunk_z;
        
        // Activate the animal
        animals[current_index].active = true;
        
        // Increment animal count
        animal_count++;
        
        logInfo("Created animal in chunk (%d,%d): type=%s, position=(%.2f, %.2f, %.2f)", 
                chunk_x, chunk_z, species->name, animals[current_index].x, 
                animals[current_index].y, animals[current_index].z);
    }
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
    
    // Use a slightly brighter color to make animals stand out
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

// Render all animals with directional sprites
void renderAnimals(float camera_x, float camera_z) {
    // For properly lit animals, we need to keep lighting enabled
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
    
    // Render each animal with the appropriate directional sprite
    for (int i = 0; i < animal_count; i++) {
        if (animals[i].active) {
            // Skip if species index is invalid
            if (animals[i].species_index < 0 || animals[i].species_index >= ANIMAL_SPECIES_COUNT) {
                continue;
            }
            
            // Calculate angle between camera and animal in the XZ plane
            float dx = camera_x - animals[i].x;
            float dz = camera_z - animals[i].z;
            
            // Calculate viewing angle in degrees (0-360)
            float view_angle = atan2f(dx, dz) * 180.0f / M_PI;
            if (view_angle < 0) view_angle += 360.0f;
            
            // Calculate the relative angle between the camera view direction and the animal's movement direction
            // This determines which of the 8 directional textures to show
            float relative_angle = view_angle - animals[i].rotation;
            
            // Normalize to 0-360 range
            while (relative_angle < 0) relative_angle += 360.0f;
            while (relative_angle >= 360.0f) relative_angle -= 360.0f;
            
            // Calculate which of the 8 direction sprites to use (45-degree segments)
            // Split the 360 degrees into 8 segments of 45 degrees each
            int direction_index = (int)((relative_angle + 22.5f) / 45.0f) % ANIMAL_DIRECTIONS;
            
            // Reverse the direction to match texture ordering
            direction_index = (ANIMAL_DIRECTIONS - 1) - direction_index;
            
            // Get the texture for this animal type and direction
            GLuint texture = animal_textures[animals[i].species_index][direction_index];
            
            // Draw the animal sprite - using fixed billboard to always face camera
            drawBillboard(
                animals[i].x, 
                animals[i].y, 
                animals[i].z, 
                animals[i].width, 
                animals[i].height, 
                texture
            );
        }
    }
    
    // Restore original material state
    glMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
}

// Clean up animal resources
void cleanupAnimals(void) {
    if (animals != NULL) {
        free(animals);
        animals = NULL;
    }
    animal_count = 0;
    animal_capacity = 0;
}

// Return a pointer to the animals array for map drawing
Animal* getAnimalsArray(void) {
    return animals;
}

// Return the current number of animals
int getAnimalCount(void) {
    return animal_count;
}

// Update animal movement and state
void updateAnimals(float delta_time) {
    // Need access to terrain for height calculations
    GameState* game = (GameState*)game_state_ptr;
    Terrain* terrain = game ? (Terrain*)game->terrain : NULL;
    
    for (int i = 0; i < animal_count; i++) {
        if (!animals[i].active) continue;
        
        // Skip if species index is invalid
        if (animals[i].species_index < 0 || animals[i].species_index >= ANIMAL_SPECIES_COUNT) {
            continue;
        }
        
        const AnimalSpecies* species = &ANIMAL_SPECIES[animals[i].species_index];
        
        // Special handling for flying animals
        if (species->behavior == ANIMAL_FLYING) {
            // Flying animals are always in motion
            animals[i].state = ANIMAL_WALKING; // Use walking state for flying
            animals[i].velocity = animals[i].max_velocity;
            
            // Calculate ground level at current position
            float ground_level = 0.0f;
            if (terrain) {
                ground_level = getHeightAtPoint(terrain, animals[i].x, animals[i].z);
            }
            
            // Handle vertical movement
            if (animals[i].state_timer <= 0.0f) {
                // Time to change vertical direction
                animals[i].ascending = !animals[i].ascending;
                animals[i].state_timer = FLYING_MIN_HEIGHT_TIME + 
                                       ((float)rand() / RAND_MAX) * 
                                       (FLYING_MAX_HEIGHT_TIME - FLYING_MIN_HEIGHT_TIME);
                
                // Randomize flight height within bounds
                animals[i].flight_height = FLYING_MIN_HEIGHT + 
                                         ((float)rand() / RAND_MAX) * 
                                         (FLYING_MAX_HEIGHT - FLYING_MIN_HEIGHT);
                                         
                // Also pick a new random direction when changing height
                float random_turn = ((float)rand() / RAND_MAX) * 180.0f - 90.0f; // ±90 degrees
                animals[i].direction += random_turn;
                while (animals[i].direction < 0) animals[i].direction += 360.0f;
                while (animals[i].direction >= 360.0f) animals[i].direction -= 360.0f;
            }
            
            // Calculate target height
            float target_height = ground_level + animals[i].flight_height;
            
            // Adjust y position based on ascending/descending
            if (animals[i].ascending) {
                // Moving up toward target height
                if (animals[i].y < target_height) {
                    animals[i].y += animals[i].vertical_velocity * delta_time;
                    // Don't exceed target height
                    if (animals[i].y > target_height) {
                        animals[i].y = target_height;
                    }
                }
            } else {
                // Moving down toward minimum height
                float min_height = ground_level + FLYING_MIN_HEIGHT;
                if (animals[i].y > min_height) {
                    animals[i].y -= animals[i].vertical_velocity * delta_time;
                    // Don't go below minimum flying height
                    if (animals[i].y < min_height) {
                        animals[i].y = min_height;
                    }
                }
            }
            
            // Calculate movement in XZ plane - ensure they always move at least a minimum amount
            float rad_direction = animals[i].direction * M_PI / 180.0f;
            float dx = sinf(rad_direction) * animals[i].velocity * delta_time;
            float dz = cosf(rad_direction) * animals[i].velocity * delta_time;
            
            // Update position - ensure a minimum movement to prevent getting stuck
            animals[i].x += dx;
            animals[i].z += dz;
            
            // Set rotation to match movement direction
            animals[i].rotation = animals[i].direction;
            
            // Check if flying animal has wandered too far
            float spawn_x = 0.0f;
            float spawn_z = 0.0f;
            
            // Use original spawn position based on index for the first 20 animals
            if (i < 10) {
                spawn_x = -10.0f + (i * 2.0f);
                spawn_z = 0.0f;
            } else if (i < 20) {
                spawn_x = -10.0f + ((i-10) * 2.0f);
                spawn_z = 5.0f;
            }
            
            // Calculate distance from spawn point
            float dist_x = animals[i].x - spawn_x;
            float dist_z = animals[i].z - spawn_z;
            float dist_sq = dist_x * dist_x + dist_z * dist_z;
            
            // If outside the wander radius, don't revert position but adjust direction to head back
            float flying_wander_radius = ANIMAL_WANDER_RADIUS * 1.5f;
            if (dist_sq > flying_wander_radius * flying_wander_radius) {
                // Calculate angle back to spawn point
                float angle_to_center = atan2f(-dist_x, -dist_z) * 180.0f / M_PI;
                
                // Add some randomness (±30 degrees) but generally head back
                float random_offset = ((float)rand() / RAND_MAX) * 60.0f - 30.0f;
                animals[i].direction = angle_to_center + random_offset;
                
                // Update rotation to match new direction
                animals[i].rotation = animals[i].direction;
            } 
            else if (rand() % 100 < 1) { // 1% chance per frame to change direction for more natural movement
                float random_turn = ((float)rand() / RAND_MAX) * 40.0f - 20.0f; // ±20 degrees
                animals[i].direction += random_turn;
                // Keep direction in 0-360 range
                while (animals[i].direction < 0) animals[i].direction += 360.0f;
                while (animals[i].direction >= 360.0f) animals[i].direction -= 360.0f;
                
                // Update rotation to match new direction
                animals[i].rotation = animals[i].direction;
            }
        } 
        else {
            // Regular ground animals
            // Update state timer
            animals[i].state_timer -= delta_time;
            
            // Check if we need to change state
            if (animals[i].state_timer <= 0.0f) {
                // Transition to next state
                if (animals[i].state == ANIMAL_IDLE) {
                    // Transition from idle to walking
                    animals[i].state = ANIMAL_WALKING;
                    
                    // Pick a random direction to walk in (can be adjusted based on behavior)
                    animals[i].direction = ((float)rand() / RAND_MAX) * 360.0f;
                    
                    // Adjust behavior based on animal type (future expansion)
                    // For now just set velocity
                    animals[i].velocity = animals[i].max_velocity;
                    
                    // Set random duration for walking state
                    animals[i].state_timer = ANIMAL_MIN_WALK_TIME + 
                                           ((float)rand() / RAND_MAX) * 
                                           (ANIMAL_MAX_WALK_TIME - ANIMAL_MIN_WALK_TIME);
                } else {
                    // Transition from walking to idle
                    animals[i].state = ANIMAL_IDLE;
                    
                    // Stop moving
                    animals[i].velocity = 0.0f;
                    
                    // Set random duration for idle state
                    animals[i].state_timer = ANIMAL_MIN_IDLE_TIME + 
                                           ((float)rand() / RAND_MAX) * 
                                           (ANIMAL_MAX_IDLE_TIME - ANIMAL_MIN_IDLE_TIME);
                }
            }
            
            // Update position based on current state
            if (animals[i].state == ANIMAL_WALKING && terrain != NULL) {
                // Calculate movement vector based on direction
                float rad_direction = animals[i].direction * M_PI / 180.0f;
                float dx = sinf(rad_direction) * animals[i].velocity * delta_time;
                float dz = cosf(rad_direction) * animals[i].velocity * delta_time;
                
                // Ensure the movement is not too small to avoid getting stuck
                const float MIN_MOVEMENT = 0.001f;
                if (fabsf(dx) < MIN_MOVEMENT && fabsf(dz) < MIN_MOVEMENT) {
                    // If movement is too small, amplify it to ensure animal keeps moving
                    float scale = MIN_MOVEMENT / fmaxf(fabsf(dx), fabsf(dz));
                    dx *= scale;
                    dz *= scale;
                }
                
                // Update position
                animals[i].x += dx;
                animals[i].z += dz;
                
                // Update Y position based on terrain height
                float ground_level = getHeightAtPoint(terrain, animals[i].x, animals[i].z);
                
                // Add a small offset to keep animal above ground
                animals[i].y = ground_level + 0.05f;
                
                // Set rotation to match movement direction (important: this is the animal's orientation)
                animals[i].rotation = animals[i].direction;
                
                // Check if animal has wandered too far from its spawn point
                float spawn_x = 0.0f;
                float spawn_z = 0.0f;
                
                // Use original spawn position based on index for the first 20 animals
                if (i < 10) {
                    spawn_x = -10.0f + (i * 2.0f);
                    spawn_z = 0.0f;
                } else if (i < 20) {
                    spawn_x = -10.0f + ((i-10) * 2.0f);
                    spawn_z = 5.0f;
                }
                
                // Calculate distance from spawn point
                float dist_x = animals[i].x - spawn_x;
                float dist_z = animals[i].z - spawn_z;
                float dist_sq = dist_x * dist_x + dist_z * dist_z;
                
                // If outside the wander radius, don't revert position but turn toward spawn point
                if (dist_sq > ANIMAL_WANDER_RADIUS * ANIMAL_WANDER_RADIUS) {
                    // Calculate angle back to spawn point
                    float angle_to_center = atan2f(-dist_x, -dist_z) * 180.0f / M_PI;
                    
                    // Add some randomness but generally head back to spawn
                    float random_offset = ((float)rand() / RAND_MAX) * 40.0f - 20.0f; // ±20 degrees
                    animals[i].direction = angle_to_center + random_offset;
                    
                    // Update rotation to match new direction
                    animals[i].rotation = animals[i].direction;
                    
                    // Boost speed slightly to help animal return to area faster
                    animals[i].velocity = animals[i].max_velocity * 1.2f;
                }
                else {
                    // Occasionally change direction for more natural movement
                    if (rand() % 250 < 1) {  // 0.4% chance per frame
                        float random_turn = ((float)rand() / RAND_MAX) * 120.0f - 60.0f; // ±60 degrees
                        animals[i].direction += random_turn;
                        
                        // Keep direction in 0-360 range
                        while (animals[i].direction < 0) animals[i].direction += 360.0f;
                        while (animals[i].direction >= 360.0f) animals[i].direction -= 360.0f;
                    }
                }
            }
        }
    }
}