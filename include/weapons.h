#ifndef WEAPONS_H
#define WEAPONS_H

#include <stdbool.h>

// Simple placeholder weapons system for FPS game
// Currently unused but kept for compatibility

#define MAX_LASERS 1  // Only define 1 laser for compatibility

// Laser state enum
typedef enum {
    LASER_INACTIVE,
    LASER_ACTIVE
} LaserState;

// Laser structure
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    bool active;
    LaserState state;
    float timer;
} Laser;

// Function declarations
void initWeapons(void);
void spawnLaser(float x, float y, float z);
void updateLasers(float delta_time);
void renderLasers(void);

#endif // WEAPONS_H