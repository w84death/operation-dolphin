#ifndef ANIMAL_TYPES_H
#define ANIMAL_TYPES_H

#include <stdbool.h>

// Enum for animal behavior types
typedef enum {
    ANIMAL_BEHAVIOR_PASSIVE,  // Doesn't react to player
    ANIMAL_BEHAVIOR_SCARED,   // Runs away from player
    ANIMAL_BEHAVIOR_AGGRESSIVE, // Attacks player,
    ANIMAL_FLYING         // Flying behavior
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
    {"Chicken",     "chicken",      0.4f,  0.4f,   0.5f,  ANIMAL_BEHAVIOR_SCARED},
    {"Dino",        "dino",         1.0f,  1.0f,   1.2f,  ANIMAL_BEHAVIOR_SCARED},
    {"T-Rex",       "trex",         3.2f,  3.2f,   1.8f,  ANIMAL_BEHAVIOR_AGGRESSIVE},
    {"Velocicaptor", "velocicaptor", 2.0f,  2.0f,   5.0f,  ANIMAL_BEHAVIOR_AGGRESSIVE},
    {"Cavesaurus", "cavesaurus",  2.0f,  2.0f,   1.5f,  ANIMAL_BEHAVIOR_AGGRESSIVE},
    {"Stegosaurus", "stegosaurus",  6.0f,  6.0f,   0.8f,  ANIMAL_BEHAVIOR_PASSIVE},
    {"Brontosaurus", "brontosaurus",2.0f,  2.0f,   1.0f,  ANIMAL_BEHAVIOR_PASSIVE},
    {"Pterodactyl", "pterodactyl",  1.5f,  1.5f,   3.0f,  ANIMAL_FLYING},
    {"ParrotBlue", "parrot_blue", 0.5f,  0.5f,   4.0f,  ANIMAL_FLYING},
    {"ParrotRed", "parrot_red", 0.5f,  0.5f,   4.0f,  ANIMAL_FLYING}
};

// Number of animal species
#define ANIMAL_SPECIES_COUNT (sizeof(ANIMAL_SPECIES) / sizeof(ANIMAL_SPECIES[0]))

#endif // ANIMAL_TYPES_H