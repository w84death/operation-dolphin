#ifndef VEGETATION_H
#define VEGETATION_H

#include <stdbool.h>
// --- Add OpenGL Extension Header ---
// If you're not using an extension loader like GLEW, 
// you might need to include specific headers for VBO functions 
// depending on your OpenGL setup. For now, assume standard GL is enough.
#include <GL/gl.h> 
// --- End Add ---
#include "config.h"
#include "player.h"

// Forward declaration of Vegetation
typedef struct Vegetation Vegetation;

// Type definition for vegetation
struct Vegetation {
    float x, y, z;       // Position
    float width, height; // Dimensions
    int texture_index;   // Which texture to use
    int type;            // 0=small, 1=medium, 2=big
    bool active;         // Whether this vegetation is visible
    int chunk_x, chunk_z; // Which chunk this vegetation belongs to
};

// Constants for max textures
#define MAX_TEXTURES_PER_SIZE 64

// Vegetation related functions
void setGameStatePointer(void* game_ptr);
bool loadVegetationTextures(void);
// --- Add VBO Init/Cleanup ---
bool initVegetationBuffers(void); 
void cleanupVegetationBuffers(void);
// --- End Add ---
void createVegetation(int count, float terrain_size);
void createVegetationForChunk(int chunk_x, int chunk_z, float chunk_size, unsigned int seed);
void renderVegetation(void);
void cutMediumFoliage(Player* player);
void cleanupVegetation(void);

#endif // VEGETATION_H