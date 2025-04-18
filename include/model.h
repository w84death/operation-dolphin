#ifndef MODEL_H
#define MODEL_H

#include <GL/gl.h>

// Structure to hold texture data
typedef struct {
    GLuint id;
    char* path;
    unsigned char* data;
    int width;
    int height;
    int channels;
} Texture;

// Structure to hold the loaded model data
typedef struct Model {
    unsigned int num_meshes;
    unsigned int* num_vertices;
    unsigned int* num_indices;
    float** vertices;
    unsigned int** indices;
    unsigned int* texture_ids;  // Texture ID for each mesh
    unsigned int num_textures;
    Texture* textures;
} Model;

// Function prototypes
Model loadModel(const char* filepath);
void freeModel(Model model);
GLuint loadTextureFromFile(const char* filename);

#endif // MODEL_H