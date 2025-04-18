#ifndef PARTICLES_H
#define PARTICLES_H

#include <stdbool.h>

// Particle life stages
typedef enum {
    PARTICLE_RED,     // Initial explosion (bright red)
    PARTICLE_WHITE,   // Middle stage (white hot)
    PARTICLE_GRAY,    // Final smoke stage (gray)
    PARTICLE_DEAD     // Particle is done
} ParticleStage;

// Structure for a single particle
typedef struct {
    float x, y, z;          // Position
    float vx, vy, vz;       // Velocity
    float size;             // Current size
    float initial_size;     // Starting size
    float life;             // Current life remaining
    float max_life;         // Maximum life time
    ParticleStage stage;    // Current stage of the particle
    bool active;            // Is the particle active?
} Particle;

// Structure for a particle effect
typedef struct {
    Particle* particles;    // Array of particles
    int count;              // Number of particles in the effect
    float x, y, z;          // Origin position of the effect
    float timer;            // Time since spawned
    bool active;            // Is the effect active?
} ParticleEffect;

// Maximum number of particle effects at once
#define MAX_PARTICLE_EFFECTS 10
// Number of particles per effect
#define PARTICLES_PER_EFFECT 15
// Life duration of each stage in seconds
#define PARTICLE_RED_DURATION 0.1f
#define PARTICLE_WHITE_DURATION 0.2f
#define PARTICLE_GRAY_DURATION 0.3f

// Function prototypes
void initParticleSystem(void);
void spawnExplosionEffect(float x, float y, float z);
void updateParticles(float delta);
void renderParticles(void);
void cleanupParticleSystem(void);

#endif // PARTICLES_H