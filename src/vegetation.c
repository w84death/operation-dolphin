#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glext.h> // Include for VBO functions (might need specific setup depending on OS/loader)
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

// --- VBO Data ---
static GLuint vegetation_vbo = 0; // Single VBO for dynamic vertex data
static GLfloat* vbo_vertex_data = NULL; // CPU buffer for vertex positions
static GLfloat* vbo_texcoord_data = NULL; // CPU buffer for texture coordinates
static int vbo_capacity_vertices = 0; // Current capacity of CPU buffers (in vertices)
// --- End VBO Data ---

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
        "textures/foliage/small/mushroom2.tga",
        "textures/foliage/small/s51.tga",
        "textures/foliage/small/s52.tga",
        "textures/foliage/small/s53.tga",
        "textures/foliage/small/s54.tga"
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
        "textures/foliage/medium/weed3.tga",
        "textures/foliage/medium/fern1.tga",
        "textures/foliage/medium/fern2.tga",
        "textures/foliage/medium/bamboo1.tga",
        "textures/foliage/medium/bamboo2.tga",
        "textures/foliage/medium/bamboo3.tga",
        "textures/foliage/medium/bamboo4.tga",
        "textures/foliage/medium/f1.tga",
        "textures/foliage/medium/f2.tga",
        "textures/foliage/medium/s25.tga",
        "textures/foliage/medium/s26.tga",
        "textures/foliage/medium/s27.tga",
        "textures/foliage/medium/s28.tga",
        "textures/foliage/medium/s33.tga",
        "textures/foliage/medium/s34.tga",
        "textures/foliage/medium/s35.tga",
        "textures/foliage/medium/s36.tga",
        "textures/foliage/medium/s37.tga",
        "textures/foliage/medium/s38.tga",
        "textures/foliage/medium/s39.tga",
        "textures/foliage/medium/s40.tga",
        "textures/foliage/medium/s41.tga",
        "textures/foliage/medium/s42.tga",
        "textures/foliage/medium/s43.tga",
        "textures/foliage/medium/s44.tga",
        "textures/foliage/medium/s45.tga",
        "textures/foliage/medium/s46.tga",
        "textures/foliage/medium/s47.tga",
        "textures/foliage/medium/s48.tga",
        "textures/foliage/medium/s49.tga",
        "textures/foliage/medium/s50.tga",
        "textures/foliage/medium/s55.tga",
        "textures/foliage/medium/s56.tga",
        "textures/foliage/medium/s57.tga",
        "textures/foliage/medium/s58.tga"
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
        "textures/foliage/big/tree5.tga",
        "textures/foliage/big/tree6.tga",
        "textures/foliage/big/tree7.tga",
        "textures/foliage/big/tree8.tga",
        "textures/foliage/big/rock2.tga",
        "textures/foliage/big/rock4.tga",
        "textures/foliage/big/oldtree1.tga",
        "textures/foliage/big/oldtree2.tga",
        "textures/foliage/big/oldtree3.tga",
        "textures/foliage/big/oldtree4.tga",
        "textures/foliage/big/oldtree5.tga",
        "textures/foliage/big/s1.tga",
        "textures/foliage/big/s2.tga",
        "textures/foliage/big/s3.tga",
        "textures/foliage/big/s4.tga",
        "textures/foliage/big/s5.tga",
        "textures/foliage/big/s6.tga",
        "textures/foliage/big/s7.tga",
        "textures/foliage/big/s8.tga",
        "textures/foliage/big/s9.tga",
        "textures/foliage/big/s10.tga",
        "textures/foliage/big/s11.tga",
        "textures/foliage/big/s12.tga",
        "textures/foliage/big/s13.tga",
        "textures/foliage/big/s14.tga",
        "textures/foliage/big/s15.tga",
        "textures/foliage/big/s16.tga",
        "textures/foliage/big/s21.tga",
        "textures/foliage/big/s22.tga",
        "textures/foliage/big/s23.tga",
        "textures/foliage/big/s24.tga",
        "textures/foliage/big/s29.tga",
        "textures/foliage/big/s30.tga",
        "textures/foliage/big/s31.tga",
        "textures/foliage/big/s32.tga"

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

// --- VBO Initialization ---
bool initVegetationBuffers() {
    // Generate the VBO
    glGenBuffers(1, &vegetation_vbo);
    if (vegetation_vbo == 0) {
        logError("Failed to generate vegetation VBO\n");
        return false;
    }
    logInfo("Vegetation VBO generated: ID %u\n", vegetation_vbo);
    
    // Initial allocation for CPU buffers (e.g., for 1000 billboards * 4 vertices)
    vbo_capacity_vertices = 4000; 
    vbo_vertex_data = malloc(vbo_capacity_vertices * 3 * sizeof(GLfloat)); // x, y, z
    vbo_texcoord_data = malloc(vbo_capacity_vertices * 2 * sizeof(GLfloat)); // u, v
    
    if (!vbo_vertex_data || !vbo_texcoord_data) {
        logError("Failed to allocate CPU buffers for vegetation VBO data\n");
        glDeleteBuffers(1, &vegetation_vbo);
        vegetation_vbo = 0;
        free(vbo_vertex_data); // free even if NULL
        free(vbo_texcoord_data);
        vbo_vertex_data = NULL;
        vbo_texcoord_data = NULL;
        vbo_capacity_vertices = 0;
        return false;
    }
    logInfo("Allocated CPU buffers for VBO (capacity: %d vertices)\n", vbo_capacity_vertices);

    return true;
}
// --- End VBO Initialization ---

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
    // Apply TERRAIN_TILES_COUNT to make vegetation cover the entire larger terrain
    createVegetationForChunk(0, 0, terrain_size * TERRAIN_TILES_COUNT, seed);
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

// --- VBO Cleanup ---
void cleanupVegetationBuffers() {
    if (vegetation_vbo != 0) {
        glDeleteBuffers(1, &vegetation_vbo);
        vegetation_vbo = 0;
        logInfo("Deleted vegetation VBO\n");
    }
    free(vbo_vertex_data);
    free(vbo_texcoord_data);
    vbo_vertex_data = NULL;
    vbo_texcoord_data = NULL;
    vbo_capacity_vertices = 0;
    logInfo("Freed CPU buffers for VBO\n");
}
// --- End VBO Cleanup ---

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

// --- REMOVE OLD drawBillboard FUNCTION ---
/* 
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture) {
    // ... (old immediate mode code removed) ...
}
*/
// --- END REMOVE ---

// --- VBO Render Function ---
void renderVegetation() {
    if (vegetation_vbo == 0 || vegetation_count == 0) {
        return; // Nothing to render or VBO not initialized
    }

    // Get camera orientation vectors from the current modelview matrix
    // These are needed to make billboards face the camera
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    // Camera Right vector (X column)
    float cam_right_x = modelview[0];
    float cam_right_y = modelview[4];
    float cam_right_z = modelview[8];
    
    // Camera Up vector (Y column) - Use the actual up vector from the matrix
    float cam_up_x = modelview[1];
    float cam_up_y = modelview[5];
    float cam_up_z = modelview[9];

    // Material setup for unshaded textures (similar to before)
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f); // Alpha test threshold
    glDepthMask(GL_TRUE); // Ensure depth writing is enabled

    // Make textures interact with lighting correctly
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Set material to ignore lighting colors but still be affected by light intensity
    GLfloat white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

    // Bind the VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vegetation_vbo);
    
    // Enable client states for vertex and texture coordinate arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // --- Batch rendering by texture --- 
    GLuint current_texture = 0; // Track the currently bound texture
    int current_batch_vertex_count = 0;

    // Helper lambda/function to render the current batch
    void render_current_batch() {
        if (current_batch_vertex_count > 0) {
            // Bind the texture for this batch
            glBindTexture(GL_TEXTURE_2D, current_texture);
            
            // Upload data for this batch to the VBO
            // We upload both position and texcoord data contiguously
            // Positions first, then texcoords
            glBufferData(GL_ARRAY_BUFFER, 
                         current_batch_vertex_count * (3 + 2) * sizeof(GLfloat), 
                         NULL, // Orphaning the buffer (performance hint)
                         GL_STREAM_DRAW); // Data changes frequently
            glBufferSubData(GL_ARRAY_BUFFER, 0, 
                            current_batch_vertex_count * 3 * sizeof(GLfloat), 
                            vbo_vertex_data);
            glBufferSubData(GL_ARRAY_BUFFER, current_batch_vertex_count * 3 * sizeof(GLfloat), 
                            current_batch_vertex_count * 2 * sizeof(GLfloat), 
                            vbo_texcoord_data);

            // Set vertex and texture coordinate pointers
            glVertexPointer(3, GL_FLOAT, 0, (void*)0); // Positions are at the start
            glTexCoordPointer(2, GL_FLOAT, 0, (void*)(current_batch_vertex_count * 3 * sizeof(GLfloat))); // Texcoords follow positions
            
            // Draw the batch
            glDrawArrays(GL_QUADS, 0, current_batch_vertex_count);
            
            // Reset batch count for the next texture
            current_batch_vertex_count = 0;
        }
    }

    // Iterate through all vegetation types (Big -> Medium -> Small for rough back-to-front)
    for (int type = 2; type >= 0; --type) {
        GLuint* textures;
        int texture_count;
        switch(type) {
            case 0: textures = vegetation_textures_small; texture_count = small_texture_count; break;
            case 1: textures = vegetation_textures_medium; texture_count = medium_texture_count; break;
            case 2: textures = vegetation_textures_big; texture_count = big_texture_count; break;
            default: continue; // Should not happen
        }

        // Iterate through each texture within the current type
        for (int tex_idx = 0; tex_idx < texture_count; ++tex_idx) {
            GLuint texture_id = textures[tex_idx];
            if (texture_id == 0) continue; // Skip unloaded textures

            // Check if this texture requires rendering the previous batch
            if (current_texture != 0 && current_texture != texture_id) {
                render_current_batch();
            }
            current_texture = texture_id;

            // Iterate through all vegetation instances to find ones using this texture
            for (int i = 0; i < vegetation_count; ++i) {
                if (!vegetation[i].active || vegetation[i].type != type || vegetation[i].texture_index != tex_idx) {
                    continue; // Skip inactive or wrong type/texture
                }

                // Check if CPU buffers need resizing
                if ((current_batch_vertex_count + 4) > vbo_capacity_vertices) {
                    // Render what we have before resizing
                    render_current_batch(); 
                    current_texture = texture_id; // Reset current texture after render

                    // Resize buffers (e.g., double the size)
                    int new_capacity = vbo_capacity_vertices * 2;
                    GLfloat* new_vertex_data = realloc(vbo_vertex_data, new_capacity * 3 * sizeof(GLfloat));
                    GLfloat* new_texcoord_data = realloc(vbo_texcoord_data, new_capacity * 2 * sizeof(GLfloat));

                    if (!new_vertex_data || !new_texcoord_data) {
                        logError("Failed to reallocate VBO CPU buffers! Rendering may be incomplete.\n");
                        // Attempt to continue with old buffers, but stop adding new data
                        if (new_vertex_data) vbo_vertex_data = new_vertex_data; // Keep whichever realloc succeeded
                        if (new_texcoord_data) vbo_texcoord_data = new_texcoord_data;
                        goto end_render_loop; // Exit loops if resize fails critically
                    }
                    vbo_vertex_data = new_vertex_data;
                    vbo_texcoord_data = new_texcoord_data;
                    vbo_capacity_vertices = new_capacity;
                    logInfo("Resized VBO CPU buffers to capacity: %d vertices\n", new_capacity);
                }

                // Calculate billboard vertices based on camera orientation
                float x = vegetation[i].x;
                float y = vegetation[i].y;
                float z = vegetation[i].z;
                float w = vegetation[i].width;
                float h = vegetation[i].height;
                float half_w = w / 2.0f;

                // Calculate the 4 corner vertices of the billboard quad
                // Bottom Left
                vbo_vertex_data[current_batch_vertex_count*3 + 0] = x - cam_right_x * half_w;
                vbo_vertex_data[current_batch_vertex_count*3 + 1] = y - cam_right_y * half_w; // Use camera right Y
                vbo_vertex_data[current_batch_vertex_count*3 + 2] = z - cam_right_z * half_w;
                vbo_texcoord_data[current_batch_vertex_count*2 + 0] = 0.0f; // U
                vbo_texcoord_data[current_batch_vertex_count*2 + 1] = 1.0f; // V (flipped)
                current_batch_vertex_count++;

                // Bottom Right
                vbo_vertex_data[current_batch_vertex_count*3 + 0] = x + cam_right_x * half_w;
                vbo_vertex_data[current_batch_vertex_count*3 + 1] = y + cam_right_y * half_w;
                vbo_vertex_data[current_batch_vertex_count*3 + 2] = z + cam_right_z * half_w;
                vbo_texcoord_data[current_batch_vertex_count*2 + 0] = 1.0f; // U
                vbo_texcoord_data[current_batch_vertex_count*2 + 1] = 1.0f; // V (flipped)
                current_batch_vertex_count++;

                // Top Right
                vbo_vertex_data[current_batch_vertex_count*3 + 0] = x + cam_right_x * half_w + cam_up_x * h;
                vbo_vertex_data[current_batch_vertex_count*3 + 1] = y + cam_right_y * half_w + cam_up_y * h;
                vbo_vertex_data[current_batch_vertex_count*3 + 2] = z + cam_right_z * half_w + cam_up_z * h;
                vbo_texcoord_data[current_batch_vertex_count*2 + 0] = 1.0f; // U
                vbo_texcoord_data[current_batch_vertex_count*2 + 1] = 0.0f; // V (flipped)
                current_batch_vertex_count++;

                // Top Left
                vbo_vertex_data[current_batch_vertex_count*3 + 0] = x - cam_right_x * half_w + cam_up_x * h;
                vbo_vertex_data[current_batch_vertex_count*3 + 1] = y - cam_right_y * half_w + cam_up_y * h;
                vbo_vertex_data[current_batch_vertex_count*3 + 2] = z - cam_right_z * half_w + cam_up_z * h;
                vbo_texcoord_data[current_batch_vertex_count*2 + 0] = 0.0f; // U
                vbo_texcoord_data[current_batch_vertex_count*2 + 1] = 0.0f; // V (flipped)
                current_batch_vertex_count++;
            }
        }
    }

    // Render the final batch if any vertices were added
    render_current_batch();

end_render_loop: // Label for exiting loops if buffer resize fails

    // --- Cleanup --- 
    // Disable client states
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Restore OpenGL state
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    
    // Restore material (optional, depends if other objects need it)
    // glMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    // glMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
}
// --- End VBO Render Function ---

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