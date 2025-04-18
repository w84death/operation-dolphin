#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <string.h>  // Added for string handling
#include "../include/terrain.h"
#include "../include/config.h"
#include "../stb_image.h"

// Create a flat terrain with a simple texture
Terrain* createFlatTerrain(float size, float height_scale) {
    Terrain* terrain = (Terrain*)malloc(sizeof(Terrain));
    if (terrain == NULL) {
        printf("Failed to allocate memory for terrain\n");
        return NULL;
    }

    // Initialize terrain properties
    terrain->size = size;
    terrain->height_scale = height_scale;
    
    // Create a simple flat grid for the terrain (21x21 vertices)
    const int resolution = 20; // Number of segments (vertices - 1)
    terrain->vertex_count = (resolution + 1) * (resolution + 1);
    terrain->index_count = resolution * resolution * 6; // 2 triangles per grid cell
    
    // Allocate memory for vertices (12 floats per vertex: position, normal, texcoord, color)
    terrain->vertices = (float*)malloc(terrain->vertex_count * 12 * sizeof(float));
    terrain->indices = (unsigned int*)malloc(terrain->index_count * sizeof(unsigned int));
    
    if (terrain->vertices == NULL || terrain->indices == NULL) {
        printf("Failed to allocate memory for terrain data\n");
        free(terrain);
        return NULL;
    }
    
    // Generate vertices
    float half_size = size / 2.0f;
    float step = size / resolution;
    int vertex_index = 0;
    
    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            float px = -half_size + x * step;
            float pz = -half_size + z * step;
            float py = 0.0f; // Flat terrain
            
            // Position (xyz)
            terrain->vertices[vertex_index++] = px;
            terrain->vertices[vertex_index++] = py;
            terrain->vertices[vertex_index++] = pz;
            
            // Normal (pointing up)
            terrain->vertices[vertex_index++] = 0.0f;
            terrain->vertices[vertex_index++] = 1.0f;
            terrain->vertices[vertex_index++] = 0.0f;
            
            // Texture coordinates
            terrain->vertices[vertex_index++] = (float)x / resolution * 5.0f;
            terrain->vertices[vertex_index++] = (float)z / resolution * 5.0f;
            
            // Color (green-brown for jungle ground)
            terrain->vertices[vertex_index++] = 0.2f;
            terrain->vertices[vertex_index++] = 0.6f;
            terrain->vertices[vertex_index++] = 0.1f;
            terrain->vertices[vertex_index++] = 1.0f;
        }
    }
    
    // Generate indices
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
    terrain->texture_id = loadTexture("textures/grass3.png");
    if (terrain->texture_id == 0) {
        printf("Warning: Failed to load terrain texture\n");
    }
    
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
    // For a flat terrain, simply return 0
    return 0.0f;
}

// Load an image file as an OpenGL texture
GLuint loadTexture(const char* filename) {
    GLuint texture_id = 0;
    int width, height, channels;
    char full_path[512];
    char* base_path = SDL_GetBasePath();

    if (!base_path) {
        printf("Error: SDL_GetBasePath() failed: %s\n", SDL_GetError());
        // Fallback: Try loading relative to the current working directory
        snprintf(full_path, sizeof(full_path), "%s", filename);
        printf("Warning: SDL_GetBasePath failed. Trying relative path from CWD: %s\n", full_path);
    } else {
        // Construct path relative to the executable
        snprintf(full_path, sizeof(full_path), "%s%s", base_path, filename);
        printf("Attempting to load texture relative to executable: %s\n", full_path);
        SDL_free(base_path); // Free the path returned by SDL
    }

    // Load image using the constructed full_path
    // Force loading as RGBA for consistency
    unsigned char* image = stbi_load(full_path, &width, &height, &channels, STBI_rgb_alpha);

    if (!image) {
        printf("Failed to load texture image: %s\n", full_path);
        // Add stbi_failure_reason() for more details
        printf("stb_image error: %s\n", stbi_failure_reason()); 
        return 0; // Return 0 if load failed
    }

    // If image loaded successfully:
    printf("Successfully loaded texture image: %s (%dx%d, %d channels originally, loaded as RGBA)\n", full_path, width, height, channels);

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

    printf("Successfully created OpenGL texture for: %s\n", full_path);

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