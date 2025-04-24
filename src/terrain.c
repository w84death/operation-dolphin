#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <string.h>  // Added for string handling
#include "../include/terrain.h"
#include "../include/config.h"
#include "../include/log.h"

// Include stb_image.h after all other includes
#define STBI_ONLY_TGA
#include "../stb_image.h"

// Global seed for terrain and vegetation generation
static unsigned int global_terrain_seed = 12345;

// Set the global terrain seed
void setGlobalTerrainSeed(unsigned int seed) {
    global_terrain_seed = seed;
    log_info("Set global terrain seed to: %u", seed);
}

// Get the global terrain seed
unsigned int getGlobalTerrainSeed(void) {
    return global_terrain_seed;
}

// Create a flat terrain with a simple texture (legacy function, kept for backward compatibility)
Terrain* createFlatTerrain(float size, float height_scale) {
    // Use default chunk (0,0) with the global seed
    return createTerrainChunk(size * TERRAIN_TILES_COUNT, height_scale, 0, 0, global_terrain_seed);
}

// Create a terrain chunk at specified coordinates using the provided seed
Terrain* createTerrainChunk(float size, float height_scale, int chunk_x, int chunk_z, unsigned int seed) {
    Terrain* terrain = (Terrain*)malloc(sizeof(Terrain));
    if (terrain == NULL) {
        log_error("Failed to allocate memory for terrain chunk (%d,%d)", chunk_x, chunk_z);
        return NULL;
    }

    // Initialize terrain properties
    terrain->size = size;
    terrain->height_scale = height_scale;
    terrain->chunk_coord.x = chunk_x;
    terrain->chunk_coord.z = chunk_z;
    
    // Create a simple flat grid for the terrain (21x21 vertices)
    const int resolution = 20; // Number of segments (vertices - 1)
    terrain->vertex_count = (resolution + 1) * (resolution + 1);
    terrain->index_count = resolution * resolution * 6; // 2 triangles per grid cell
    
    // Allocate memory for vertices (12 floats per vertex: position, normal, texcoord, color)
    terrain->vertices = (float*)malloc(terrain->vertex_count * 12 * sizeof(float));
    terrain->indices = (unsigned int*)malloc(terrain->index_count * sizeof(unsigned int));
    
    if (terrain->vertices == NULL || terrain->indices == NULL) {
        log_error("Failed to allocate memory for terrain data");
        free(terrain);
        return NULL;
    }
    
    // Initialize the random number generator with a seed derived from global seed and chunk coordinates
    // This ensures each chunk gets a unique but deterministic terrain pattern
    unsigned int chunk_seed = seed + (unsigned int)((chunk_x * 73856093) ^ (chunk_z * 19349663));
    srand(chunk_seed);
    
    // Generate vertices
    float half_size = size / 2.0f;
    float step = size / resolution;
    int vertex_index = 0;
    
    // Calculate world position offset for this chunk
    float chunk_offset_x = chunk_x * size;
    float chunk_offset_z = chunk_z * size;
    
    // Height randomization constants
    const float max_height_variation = height_scale * 0.5f; // Maximum height variation
    
    // Temporary array to store height values for normal calculations
    float* height_map = (float*)malloc((resolution + 1) * (resolution + 1) * sizeof(float));
    if (height_map == NULL) {
        log_error("Failed to allocate memory for height map");
        free(terrain->vertices);
        free(terrain->indices);
        free(terrain);
        return NULL;
    }
    
    // First pass: Generate positions and store heights
    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            int idx = z * (resolution + 1) + x;
            float px = -half_size + x * step + chunk_offset_x;
            float pz = -half_size + z * step + chunk_offset_z;
            
            // Default flat terrain
            float py = 0.0f;
            
            // Apply height randomization only to non-border vertices
            // This keeps borders flat for smooth transitions between chunks
            if (x > 0 && x < resolution && z > 0 && z < resolution) {
                // Random height variation, biased toward smaller changes
                float random_value = (float)rand() / RAND_MAX;
                float height_variation = random_value * random_value * max_height_variation;
                
                // Apply some noise to make the terrain less uniform
                // The closer to the edge, the smaller the variation to create a smooth gradient to borders
                float edge_factor = fminf(
                    fminf((float)x / (resolution / 4.0f), (float)(resolution - x) / (resolution / 4.0f)),
                    fminf((float)z / (resolution / 4.0f), (float)(resolution - z) / (resolution / 4.0f))
                );
                edge_factor = fminf(1.0f, edge_factor); // Clamp to 1.0
                
                py = height_variation * edge_factor;
            }
            
            // Store height in temporary array for normal calculations
            height_map[idx] = py;
            
            // Position (xyz) - Store in the vertex array
            terrain->vertices[vertex_index++] = px;
            terrain->vertices[vertex_index++] = py;
            terrain->vertices[vertex_index++] = pz;
            
            // Placeholder for normal (will be calculated in second pass)
            vertex_index += 3;
            
            // Texture coordinates
            terrain->vertices[vertex_index++] = (float)x / resolution * 5.0f;
            terrain->vertices[vertex_index++] = (float)z / resolution * 5.0f;
            
            // Color with slight variation based on height
            float green = 0.6f - py * 0.2f; // Less green at higher elevations
            float brown = 0.2f + py * 0.4f; // More brown at higher elevations
            terrain->vertices[vertex_index++] = brown;
            terrain->vertices[vertex_index++] = green;
            terrain->vertices[vertex_index++] = 0.1f; 
            terrain->vertices[vertex_index++] = 1.0f;
        }
    }
    
    // Second pass: Calculate normals
    vertex_index = 0;
    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            // Skip position
            vertex_index += 3;
            
            // Calculate normal using central differences
            float nx = 0.0f, ny = 1.0f, nz = 0.0f;
            
            if (x > 0 && x < resolution && z > 0 && z < resolution) {
                // Sample heights from neighbors
                float h_left = height_map[z * (resolution + 1) + (x - 1)];
                float h_right = height_map[z * (resolution + 1) + (x + 1)];
                float h_up = height_map[(z - 1) * (resolution + 1) + x];
                float h_down = height_map[(z + 1) * (resolution + 1) + x];
                
                // Calculate normal using central differences
                nx = h_left - h_right;
                nz = h_up - h_down;
                
                // Normalize the normal vector
                float len = sqrtf(nx * nx + 1.0f + nz * nz);
                nx /= len;
                ny /= len;
                nz /= len;
            }
            
            // Store normal
            terrain->vertices[vertex_index++] = nx;
            terrain->vertices[vertex_index++] = ny;
            terrain->vertices[vertex_index++] = nz;
            
            // Skip texture coordinates and color
            vertex_index += 6;
        }
    }
    
    // Free the temporary height map
    free(height_map);
    
    // Generate indices (unchanged)
    int index_index = 0;
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            int top_left = z * (resolution + 1) + x;
            int top_right = top_left + 1;
            int bottom_left = (z + 1) * (resolution + 1) + x;
            int bottom_right = bottom_left + 1;
            
            // First triangle (top-left, bottom-left, bottom-right)
            terrain->indices[index_index++] = top_left;
            terrain->indices[index_index++] = bottom_left;
            terrain->indices[index_index++] = bottom_right;
            
            // Second triangle (top-left, bottom-right, top-right)
            terrain->indices[index_index++] = top_left;
            terrain->indices[index_index++] = bottom_right;
            terrain->indices[index_index++] = top_right;
        }
    }
    
    // Create OpenGL buffers
    glGenBuffers(1, &terrain->vertex_buffer);
    glGenBuffers(1, &terrain->index_buffer);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, terrain->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 
                 terrain->vertex_count * 12 * sizeof(float), 
                 terrain->vertices, 
                 GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 terrain->index_count * sizeof(unsigned int), 
                 terrain->indices, 
                 GL_STATIC_DRAW);
    
    // Load a ground texture
    terrain->texture_id = loadTexture("textures/terrain/ground.tga");
    if (terrain->texture_id == 0) {
        log_warning("Warning: Failed to load terrain texture");
    }
    
    // Restore the global random seed to prevent affecting other randomized elements
    srand(seed);
    
    log_info("Created terrain chunk at (%d,%d) with seed %u and height variations", chunk_x, chunk_z, seed);
    
    return terrain;
}

// Render the terrain
void renderTerrain(Terrain* terrain) {
    if (terrain == NULL) return;
    
    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, terrain->texture_id);
    
    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, terrain->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->index_buffer);
    
    // Set up vertex attributes
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 12 * sizeof(float), (void*)0);
    glNormalPointer(GL_FLOAT, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glColorPointer(4, GL_FLOAT, 12 * sizeof(float), (void*)(8 * sizeof(float)));
    
    // Draw terrain
    glDrawElements(GL_TRIANGLES, terrain->index_count, GL_UNSIGNED_INT, 0);
    
    // Clean up
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisable(GL_TEXTURE_2D);
}

// Get height at a specific point on the terrain
float getHeightAtPoint(Terrain* terrain, float x, float z) {
    if (terrain == NULL) return 0.0f;
    
    // Adjust coordinates to be relative to this chunk
    float local_x = x - terrain->chunk_coord.x * terrain->size;
    float local_z = z - terrain->chunk_coord.z * terrain->size;
    
    // Convert to grid coordinates
    float half_size = terrain->size / 2.0f;
    local_x += half_size;
    local_z += half_size;
    
    // Grid resolution
    const int resolution = 20;
    float step = terrain->size / resolution;
    
    // Get grid cell indices
    int grid_x = (int)(local_x / step);
    int grid_z = (int)(local_z / step);
    
    // Clamp to valid range
    if (grid_x < 0) grid_x = 0;
    if (grid_z < 0) grid_z = 0;
    if (grid_x >= resolution) grid_x = resolution - 1;
    if (grid_z >= resolution) grid_z = resolution - 1;
    
    // Get the four corners of the grid cell
    int v00 = grid_z * (resolution + 1) + grid_x;         // Top-left
    int v01 = grid_z * (resolution + 1) + (grid_x + 1);   // Top-right
    int v10 = (grid_z + 1) * (resolution + 1) + grid_x;   // Bottom-left
    int v11 = (grid_z + 1) * (resolution + 1) + (grid_x + 1); // Bottom-right
    
    // Get the heights from the vertex data
    float h00 = terrain->vertices[v00 * 12 + 1];
    float h01 = terrain->vertices[v01 * 12 + 1];
    float h10 = terrain->vertices[v10 * 12 + 1];
    float h11 = terrain->vertices[v11 * 12 + 1];
    
    // Calculate the position within the cell (0.0-1.0)
    float cell_x = (local_x - grid_x * step) / step;
    float cell_z = (local_z - grid_z * step) / step;
    
    // Interpolate height using bilinear interpolation
    float top = h00 * (1.0f - cell_x) + h01 * cell_x;
    float bottom = h10 * (1.0f - cell_x) + h11 * cell_x;
    float height = top * (1.0f - cell_z) + bottom * cell_z;
    
    return height;
}

// Load an image file as an OpenGL texture
GLuint loadTexture(const char* filename) {
    GLuint texture_id = 0;
    int width, height, channels;
    char full_path[512];
    char* base_path = SDL_GetBasePath();

    if (!base_path) {
        log_error("Error: SDL_GetBasePath() failed: %s", SDL_GetError());
        // Fallback: Try loading relative to the current working directory
        snprintf(full_path, sizeof(full_path), "%s", filename);
        log_warning("Warning: SDL_GetBasePath failed. Trying relative path from CWD: %s", full_path);
    } else {
        // Construct path relative to the executable
        snprintf(full_path, sizeof(full_path), "%s%s", base_path, filename);
        log_info("Attempting to load texture relative to executable: %s", full_path);
        SDL_free(base_path); // Free the path returned by SDL
    }

    // Load image using the constructed full_path
    // Force loading as RGBA for consistency
    unsigned char* image = stbi_load(full_path, &width, &height, &channels, STBI_rgb_alpha);

    if (!image) {
        log_error("Failed to load texture image: %s", full_path);
        // Add stbi_failure_reason() for more details
        log_error("stb_image error: %s", stbi_failure_reason()); 
        return 0; // Return 0 if load failed
    }

    // If image loaded successfully:
    log_info("Successfully loaded texture image: %s (%dx%d, %d channels originally, loaded as RGBA)", full_path, width, height, channels);

    // Create OpenGL texture
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // Free image data
    stbi_image_free(image);

    log_info("Successfully created OpenGL texture for: %s", full_path);

    return texture_id;
}

// Clean up terrain resources
void cleanupTerrain(Terrain* terrain) {
    if (terrain == NULL) return;
    
    // Free OpenGL resources
    if (terrain->vertex_buffer != 0) {
        glDeleteBuffers(1, &terrain->vertex_buffer);
    }
    
    if (terrain->index_buffer != 0) {
        glDeleteBuffers(1, &terrain->index_buffer);
    }
    
    if (terrain->texture_id != 0) {
        glDeleteTextures(1, &terrain->texture_id);
    }
    
    // Free memory
    free(terrain->vertices);
    free(terrain->indices);
    free(terrain);
}