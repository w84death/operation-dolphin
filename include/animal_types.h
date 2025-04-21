#ifndef ANIMAL_TYPES_H
#define ANIMAL_TYPES_H

#include <stdbool.h>

// Enum for animal behavior types
typedef enum {
    ANIMAL_BEHAVIOR_PASSIVE,  // Doesn't react to player
    ANIMAL_BEHAVIOR_SCARED,   // Runs away from player
    ANIMAL_BEHAVIOR_AGGRESSIVE // Attacks player
} AnimalBehaviorType;

// Structure to define animal species properties
typedef struct {
    const char* name;          // Animal name
    const char* folder_name;   // Folder name containing sprites
    float width;               // Base width in meters
    float height;              // Base height in meters
    float speed;               // Base speed (units per second)
    AnimalBehaviorType behavior; // Behavior type
} AnimalSpecies;

// List of all animal species in the game
static const AnimalSpecies ANIMAL_SPECIES[] = {
    // name,       folder_name, width, height, speed, behavior
    {"Chicken",    "chicken",   0.5f,  0.6f,   0.5f,  ANIMAL_BEHAVIOR_SCARED},
    {"Dino",       "dino",      1.0f,  1.5f,   1.2f,  ANIMAL_BEHAVIOR_AGGRESSIVE},
    {"Rabbit",     "rabbit",    0.4f,  0.5f,   1.8f,  ANIMAL_BEHAVIOR_SCARED},
    {"Bear",       "bear",      1.5f,  2.2f,   1.0f,  ANIMAL_BEHAVIOR_AGGRESSIVE},
    {"Deer",       "deer",      1.2f,  1.8f,   1.5f,  ANIMAL_BEHAVIOR_SCARED},
    {"Fox",        "fox",       0.6f,  0.7f,   1.3f,  ANIMAL_BEHAVIOR_PASSIVE},
    {"Wolf",       "wolf",      0.9f,  1.0f,   1.6f,  ANIMAL_BEHAVIOR_AGGRESSIVE},
    {"Turtle",     "turtle",    0.5f,  0.3f,   0.3f,  ANIMAL_BEHAVIOR_PASSIVE}
};

// Number of animal species
#define ANIMAL_SPECIES_COUNT (sizeof(ANIMAL_SPECIES) / sizeof(ANIMAL_SPECIES[0]))

#endif // ANIMAL_TYPES_H