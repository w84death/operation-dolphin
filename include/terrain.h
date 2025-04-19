#ifndef TERRAIN_H
#define TERRAIN_H

#include <stdbool.h>
#include <GL/gl.h>

// Chunk coordinates
typedef struct ChunkCoord {
    int x;
    int z;
} ChunkCoord;

// Terrain structure
typedef struct Terrain {
    float size;          // Size of the terrain
    float height_scale;  // Scale for terrain height
    
    // Chunk information
    ChunkCoord chunk_coord;  // Which chunk this terrain represents
    
    // OpenGL data
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint texture_id;
    
    // Mesh data
    float* vertices;     // Vertex data (position, normal, texcoord, color)
    unsigned int* indices;
    int vertex_count;
    int index_count;
} Terrain;

// Function declarations
Terrain* createFlatTerrain(float size, float height_scale);
Terrain* createTerrainChunk(float size, float height_scale, int chunk_x, int chunk_z, unsigned int seed);
void renderTerrain(Terrain* terrain);
float getHeightAtPoint(Terrain* terrain, float x, float z);
void cleanupTerrain(Terrain* terrain);
GLuint loadTexture(const char* filename);

// Global seed management
void setGlobalTerrainSeed(unsigned int seed);
unsigned int getGlobalTerrainSeed(void);

#endif // TERRAIN_H