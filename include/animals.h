#ifndef ANIMALS_H
#define ANIMALS_H

#include <stdbool.h>
#include <GL/gl.h>
#include "config.h"

// Forward declaration of Animal
typedef struct Animal Animal;

// Enum for animal types
typedef enum {
    ANIMAL_CHICKEN,
    ANIMAL_DINO,
    ANIMAL_COUNT
} AnimalType;

// Number of directional sprites per animal
#define ANIMAL_DIRECTIONS 8

// Type definition for Animal
struct Animal {
    float x, y, z;       // Position
    float width, height; // Dimensions
    float rotation;      // Rotation in degrees (0-360)
    AnimalType type;     // Which animal type (chicken, dino, etc.)
    bool active;         // Whether this animal is visible
    int chunk_x, chunk_z; // Which chunk this animal belongs to
};

// Constants for animal textures and rendering
#define MAX_ANIMAL_TYPES 16
#define MAX_ANIMAL_COUNT 500

// Animal related functions
void setAnimalGameStatePointer(void* game_ptr);
bool loadAnimalTextures(void);
void createAnimals(int count, float terrain_size);
void renderAnimals(float camera_x, float camera_z);
void cleanupAnimals(void);

#endif // ANIMALS_H