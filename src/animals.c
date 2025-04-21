#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include "../include/animals.h"
#include "../include/terrain.h"
#include "../include/log.h"
#include "../include/game.h"
#include "../stb_image.h" // Add stb_image.h for texture loading

// Global pointer to current game state
static void* game_state_ptr = NULL;

// Global animals data
static Animal* animals = NULL;
static int animal_count = 0;
static int animal_capacity = 0;

// Textures for animal sprites (for each type and each direction)
static GLuint animal_textures[MAX_ANIMAL_TYPES][ANIMAL_DIRECTIONS];
static bool animal_textures_loaded[MAX_ANIMAL_TYPES] = {false};
static const char* animal_type_names[MAX_ANIMAL_TYPES] = {"chicken", "dino"};

// Set the game state pointer for settings access
void setAnimalGameStatePointer(void* game_ptr) {
    game_state_ptr = game_ptr;
}

// Load textures for all animal types
bool loadAnimalTextures() {
    // Define paths for each animal type (each with 8 directional sprites)
    const char* animal_texture_paths[ANIMAL_COUNT][ANIMAL_DIRECTIONS] = {
        // Chicken textures (8 directions)
        {
            "textures/animals/chicken/1.tga",
            "textures/animals/chicken/2.tga",
            "textures/animals/chicken/3.tga",
            "textures/animals/chicken/4.tga",
            "textures/animals/chicken/5.tga",
            "textures/animals/chicken/6.tga",
            "textures/animals/chicken/7.tga",
            "textures/animals/chicken/1.tga"  // Reuse first texture for 8th direction to complete the circle
        },
        // Dino textures (8 directions)
        {
            "textures/animals/dino/1.tga",
            "textures/animals/dino/2.tga",
            "textures/animals/dino/3.tga",
            "textures/animals/dino/4.tga",
            "textures/animals/dino/5.tga",
            "textures/animals/dino/6.tga",
            "textures/animals/dino/7.tga",
            "textures/animals/dino/1.tga"  // Reuse first texture for 8th direction to complete the circle
        }
    };

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

    // Load textures for each animal type and direction
    for (int type = 0; type < ANIMAL_COUNT; type++) {
        bool all_loaded = true;
        
        for (int dir = 0; dir < ANIMAL_DIRECTIONS; dir++) {
            animal_textures[type][dir] = loadTexture(animal_texture_paths[type][dir]);
            
            if (animal_textures[type][dir] == 0) {
                logWarning("Failed to load texture for animal type %s, direction %d: %s", 
                          animal_type_names[type], dir, animal_texture_paths[type][dir]);
                all_loaded = false;
            }
        }
        
        animal_textures_loaded[type] = all_loaded;
        
        if (all_loaded) {
            logInfo("Successfully loaded all textures for animal type: %s", animal_type_names[type]);
        } else {
            logWarning("Not all textures were loaded for animal type: %s", animal_type_names[type]);
        }
    }

    // Return true if at least one animal type was fully loaded
    for (int i = 0; i < ANIMAL_COUNT; i++) {
        if (animal_textures_loaded[i]) {
            return true;
        }
    }
    
    logError("Failed to load any complete set of animal textures");
    return false;
}

// Function to ensure we have enough capacity for animals
static void ensureAnimalCapacity(int required_capacity) {
    if (animal_capacity >= required_capacity) {
        return; // We already have enough capacity
    }
    
    // Calculate new capacity (double current or minimum required)
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
    
    // Make sure we have enough capacity
    ensureAnimalCapacity(count);
    
    // Calculate world position
    float half_size = terrain_size / 2.0f;
    float terrain_offset_x = TERRAIN_POSITION_X;
    float terrain_offset_z = TERRAIN_POSITION_Z;
    float ground_level = TERRAIN_POSITION_Y;
    
    // Create animals
    for (int i = 0; i < count && i < animal_capacity; i++) {
        // Randomly select animal type
        animals[i].type = rand() % ANIMAL_COUNT;
        
        if (!animal_textures_loaded[animals[i].type]) {
            // Skip this animal if its textures aren't loaded
            continue;
        }
        
        // For debugging purposes, place animals in a grid pattern near origin
        // This makes them easier to find during testing
        if (i < 10) {
            // Place first 10 animals in a line at Z=0, starting at X=-10 with 2m spacing
            animals[i].x = -10.0f + (i * 2.0f);
            animals[i].z = 0.0f;
        } else if (i < 20) {
            // Place next 10 animals in a line at Z=5, starting at X=-10 with 2m spacing
            animals[i].x = -10.0f + ((i-10) * 2.0f);
            animals[i].z = 5.0f;
        } else {
            // Place remaining animals randomly
            animals[i].x = ((float)rand() / RAND_MAX) * terrain_size - half_size + terrain_offset_x;
            animals[i].z = ((float)rand() / RAND_MAX) * terrain_size - half_size + terrain_offset_z;
        }
        
        // Initialize movement parameters
        animals[i].rotation = ((float)rand() / RAND_MAX) * 360.0f;
        animals[i].direction = animals[i].rotation; // Initial movement direction matches rotation
        animals[i].velocity = 0.0f; // Start idle
        animals[i].state = ANIMAL_IDLE;
        
        // Set random state timer for first state transition
        animals[i].state_timer = ANIMAL_MIN_IDLE_TIME + 
                                ((float)rand() / RAND_MAX) * (ANIMAL_MAX_IDLE_TIME - ANIMAL_MIN_IDLE_TIME);
        
        // Set maximum velocity based on animal type
        if (animals[i].type == ANIMAL_CHICKEN) {
            animals[i].max_velocity = ANIMAL_CHICKEN_SPEED;
        } else if (animals[i].type == ANIMAL_DINO) {
            animals[i].max_velocity = ANIMAL_DINO_SPEED;
        }
        
        // Set dimensions based on animal type - INCREASE SIZE FOR EASIER VISIBILITY
        if (animals[i].type == ANIMAL_CHICKEN) {
            animals[i].width = 1.0f + ((float)rand() / RAND_MAX) * 0.2f;  // 1.0-1.2m (was 0.5-0.7m)
            animals[i].height = 1.2f + ((float)rand() / RAND_MAX) * 0.2f; // 1.2-1.4m (was 0.6-0.8m)
        } else if (animals[i].type == ANIMAL_DINO) {
            animals[i].width = 2.0f + ((float)rand() / RAND_MAX) * 0.3f;  // 2.0-2.3m (was 1.0-1.3m)
            animals[i].height = 3.0f + ((float)rand() / RAND_MAX) * 0.5f; // 3.0-3.5m (was 1.5-2.0m)
        }
        
        // Position on ground level - RAISE SLIGHTLY HIGHER
        animals[i].y = ground_level + 0.05f;  // Raised from 0.01f to 0.05f to avoid z-fighting
        
        // Chunk coordinates (for future use with chunks)
        animals[i].chunk_x = 0;
        animals[i].chunk_z = 0;
        
        // Activate the animal
        animals[i].active = true;
        
        animal_count++;
        
        logInfo("Created animal %d: type=%s, position=(%.2f, %.2f, %.2f), size=%.2fx%.2f", 
                i, animal_type_names[animals[i].type], 
                animals[i].x, animals[i].y, animals[i].z, 
                animals[i].width, animals[i].height);
    }
    
    logInfo("Created %d animals on the terrain", animal_count);
}

// Draw a billboard that always faces the camera (similar to vegetation system)
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

// Draw an animal sprite facing a specific direction
static void drawAnimalSprite(float x, float y, float z, float width, float height, 
                            GLuint texture, float rotation) {
    // Only draw if we have a valid texture
    if (texture == 0) return;
    
    // Save the current matrix
    glPushMatrix();
    
    // Position at the base of the sprite
    glTranslatef(x, y, z);
    
    // Get the current modelview matrix
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    // Extract camera right and up vectors to create a billboard that faces the camera
    // but keeps animal rotation - create a rotation matrix
    
    // Modified billboard matrix that preserves vertical Y axis
    // but makes animal face the camera in XZ plane
    float billboardMatrix[16];
    // Copy original matrix to preserve translation
    for (int i = 0; i < 16; i++) {
        billboardMatrix[i] = modelview[i];
    }
    
    // Clear rotation components in XZ plane but preserve Y rotation
    billboardMatrix[0] = cosf(rotation * M_PI / 180.0f);  // Custom rotation
    billboardMatrix[2] = -sinf(rotation * M_PI / 180.0f);  // Custom rotation
    billboardMatrix[8] = sinf(rotation * M_PI / 180.0f);  // Custom rotation
    billboardMatrix[10] = cosf(rotation * M_PI / 180.0f);  // Custom rotation
    
    // Keep Y axis vertical
    billboardMatrix[1] = 0.0f;
    billboardMatrix[4] = 0.0f;
    billboardMatrix[5] = 1.0f;
    billboardMatrix[6] = 0.0f;
    billboardMatrix[9] = 0.0f;
    
    // Apply the modified matrix
    glLoadMatrixf(billboardMatrix);
    
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
    
    // Debug visual - add color tinting to make animals stand out during debugging
    glColor4f(1.2f, 1.2f, 1.0f, 1.0f); // Slightly yellowish tint to make them stand out
    
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
            GLuint texture = animal_textures[animals[i].type][direction_index];
            
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

// Update animal movement and state
void updateAnimals(float delta_time) {
    // Need access to terrain for height calculations
    GameState* game = (GameState*)game_state_ptr;
    Terrain* terrain = game ? (Terrain*)game->terrain : NULL;
    
    for (int i = 0; i < animal_count; i++) {
        if (!animals[i].active) continue;
        
        // Update state timer
        animals[i].state_timer -= delta_time;
        
        // Check if we need to change state
        if (animals[i].state_timer <= 0.0f) {
            // Transition to next state
            if (animals[i].state == ANIMAL_IDLE) {
                // Transition from idle to walking
                animals[i].state = ANIMAL_WALKING;
                
                // Pick a random direction to walk in
                animals[i].direction = ((float)rand() / RAND_MAX) * 360.0f;
                
                // Set full velocity for movement
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
            
            // Store original position in case we need to revert
            float original_x = animals[i].x;
            float original_z = animals[i].z;
            float original_y = animals[i].y;
            
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
            // For simplicity we're using a circular boundary with radius ANIMAL_WANDER_RADIUS
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
            
            // If outside the wander radius, revert movement and pick a new direction
            if (dist_sq > ANIMAL_WANDER_RADIUS * ANIMAL_WANDER_RADIUS) {
                // Revert position
                animals[i].x = original_x;
                animals[i].y = original_y;
                animals[i].z = original_z;
                
                // Pick new direction (towards spawn point)
                float angle_to_center = atan2f(-dist_x, -dist_z) * 180.0f / M_PI;
                
                // Add some randomness to the angle (±45 degrees)
                float random_offset = ((float)rand() / RAND_MAX) * 90.0f - 45.0f;
                animals[i].direction = angle_to_center + random_offset;
                
                // Update rotation to match new direction
                animals[i].rotation = animals[i].direction;
            }
        }
    }
}