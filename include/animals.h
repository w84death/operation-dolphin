#ifndef ANIMALS_H
#define ANIMALS_H

#include <stdbool.h>
#include <GL/gl.h>
#include "config.h"
#include "animal_types.h"

// Forward declaration of Animal
typedef struct Animal Animal;

// Enum for animal movement states
typedef enum {
    ANIMAL_IDLE,
    ANIMAL_WALKING,
    ANIMAL_COUNT_STATES
} AnimalState;

// Number of directional sprites per animal
#define ANIMAL_DIRECTIONS 8

// Type definition for Animal
struct Animal {
    float x, y, z;       // Position
    float width, height; // Dimensions
    float rotation;      // Rotation in degrees (0-360)
    float velocity;      // Current movement speed
    float direction;     // Movement direction in degrees (0-360)
    int species_index;   // Index into ANIMAL_SPECIES array
    AnimalState state;   // Current movement state (idle, walking)
    bool active;         // Whether this animal is visible
    int chunk_x, chunk_z; // Which chunk this animal belongs to
    
    // Movement control
    float state_timer;   // Time remaining in current state
    float max_velocity;  // Maximum movement speed
    
    // Flying animal specific fields
    float flight_height; // Flying animals' target height above the terrain
    float vertical_velocity; // Vertical movement speed
    bool ascending;      // Whether the animal is currently ascending
};


// Animal related functions
void setAnimalGameStatePointer(void* game_ptr);
bool loadAnimalTextures(void);
void createAnimals(int count, float terrain_size);
void createAnimalsForChunk(int chunk_x, int chunk_z, float chunk_size, unsigned int seed, int count, float flying_ratio);
void updateAnimals(float delta_time);
void renderAnimals(float camera_x, float camera_z);
void cleanupAnimals(void);

// Functions to access animal data from other modules
Animal* getAnimalsArray(void);
int getAnimalCount(void);

#endif // ANIMALS_H