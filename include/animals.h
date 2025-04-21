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
};

// Constants for animal textures and rendering
#define MAX_ANIMAL_SPECIES 16
#define MAX_ANIMAL_COUNT 500

// Animal movement constants
#define ANIMAL_MIN_IDLE_TIME 1.0f     // Minimum idle time in seconds
#define ANIMAL_MAX_IDLE_TIME 5.0f     // Maximum idle time in seconds
#define ANIMAL_MIN_WALK_TIME 2.0f     // Minimum walking time in seconds
#define ANIMAL_MAX_WALK_TIME 8.0f     // Maximum walking time in seconds
#define ANIMAL_WANDER_RADIUS 15.0f    // Maximum distance animals can wander from spawn point

// Animal related functions
void setAnimalGameStatePointer(void* game_ptr);
bool loadAnimalTextures(void);
void createAnimals(int count, float terrain_size);
void updateAnimals(float delta_time);
void renderAnimals(float camera_x, float camera_z);
void cleanupAnimals(void);

#endif // ANIMALS_H