#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "../include/particles.h"

// Global particle effects array
ParticleEffect effects[MAX_PARTICLE_EFFECTS];

// Initialize the particle system
void initParticleSystem(void) {
    // Initialize all effects as inactive
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        effects[i].active = false;
        effects[i].particles = NULL;
        effects[i].count = 0;
    }
}

// Spawn a new explosion effect at the given coordinates
void spawnExplosionEffect(float x, float y, float z) {
    // Find an inactive effect slot
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        if (!effects[i].active) {
            // Initialize the effect
            effects[i].active = true;
            effects[i].timer = 0.0f;
            effects[i].x = x;
            effects[i].y = y;
            effects[i].z = z;
            effects[i].count = PARTICLES_PER_EFFECT;
            
            // Allocate memory for particles
            effects[i].particles = (Particle*)malloc(effects[i].count * sizeof(Particle));
            
            // Initialize each particle
            for (int j = 0; j < effects[i].count; j++) {
                Particle* p = &effects[i].particles[j];
                p->active = true;
                p->stage = PARTICLE_RED;
                
                // Random position near the explosion center (small initial offset)
                p->x = x + ((float)rand() / RAND_MAX * 0.4f - 0.2f);
                p->y = y + ((float)rand() / RAND_MAX * 0.4f - 0.2f);
                p->z = z + ((float)rand() / RAND_MAX * 0.4f - 0.2f);
                
                // Random velocity in all directions
                // More upward than downward bias
                p->vx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 2.0f; 
                p->vy = ((float)rand() / RAND_MAX * 1.5f + 0.5f) * 2.0f;
                p->vz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 2.0f;
                
                // Random size
                p->initial_size = 0.05f + (float)rand() / RAND_MAX * 0.1f;
                p->size = p->initial_size;
                
                // Lifetime
                p->max_life = PARTICLE_RED_DURATION + PARTICLE_WHITE_DURATION + PARTICLE_GRAY_DURATION;
                p->max_life *= 0.8f + (float)rand() / RAND_MAX * 0.4f; // Some variance in lifetime
                p->life = p->max_life;
            }
            
            return; // Effect spawned
        }
    }
}

// Update all particle effects
void updateParticles(float delta) {
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        if (!effects[i].active) continue;
        
        effects[i].timer += delta;
        
        bool all_particles_dead = true;
        
        for (int j = 0; j < effects[i].count; j++) {
            Particle* p = &effects[i].particles[j];
            if (!p->active) continue;
            
            // Reduce lifetime
            p->life -= delta;
            
            // Check if particle should transition to next stage or die
            if (p->life <= 0.0f) {
                p->active = false;
                continue;
            } else if (p->life <= p->max_life - PARTICLE_RED_DURATION - PARTICLE_WHITE_DURATION) {
                p->stage = PARTICLE_GRAY;
            } else if (p->life <= p->max_life - PARTICLE_RED_DURATION) {
                p->stage = PARTICLE_WHITE;
            }
            
            // Update position based on velocity
            p->x += p->vx * delta;
            p->y += p->vy * delta;
            p->z += p->vz * delta;
            
            // Add gravity effect
            p->vy -= 2.0f * delta;
            
            // Scale size based on lifetime
            if (p->stage == PARTICLE_RED) {
                // Expand in red phase
                p->size = p->initial_size * (1.0f + (PARTICLE_RED_DURATION - p->life) / PARTICLE_RED_DURATION);
            } else if (p->stage == PARTICLE_WHITE) {
                // Maintain size in white phase
                p->size = p->initial_size * 2.0f; 
            } else {
                // Expand further in gray (smoke) phase
                float gray_progress = 1.0f - (p->life / PARTICLE_GRAY_DURATION);
                p->size = p->initial_size * (2.0f + gray_progress * 1.5f);
            }
            
            // Slow down particles over time
            p->vx *= 0.95f;
            p->vy *= 0.95f;
            p->vz *= 0.95f;
            
            all_particles_dead = false;
        }
        
        // If all particles are dead, clean up this effect
        if (all_particles_dead) {
            free(effects[i].particles);
            effects[i].particles = NULL;
            effects[i].active = false;
        }
    }
}

// Render all active particle effects
void renderParticles(void) {
    // Disable lighting and texturing for particles
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        if (!effects[i].active) continue;
        
        for (int j = 0; j < effects[i].count; j++) {
            Particle* p = &effects[i].particles[j];
            if (!p->active) continue;
            
            glPushMatrix();
            glTranslatef(p->x, p->y, p->z);
            
            // Set color based on particle stage
            if (p->stage == PARTICLE_RED) {
                // Bright red to orange
                glColor4f(1.0f, 0.3f, 0.1f, 0.9f);
            } else if (p->stage == PARTICLE_WHITE) {
                // White to yellowish
                glColor4f(1.0f, 0.9f, 0.8f, 0.8f);
            } else {
                // Gray smoke with fading alpha
                float life_ratio = p->life / PARTICLE_GRAY_DURATION;
                glColor4f(0.6f, 0.6f, 0.6f, life_ratio * 0.7f);
            }
            
            // Draw a simple cube for the particle
            float half_size = p->size / 2.0f;
            
            glBegin(GL_QUADS);
            
            // Front face
            glVertex3f(-half_size, -half_size, half_size);
            glVertex3f(half_size, -half_size, half_size);
            glVertex3f(half_size, half_size, half_size);
            glVertex3f(-half_size, half_size, half_size);
            
            // Back face
            glVertex3f(-half_size, -half_size, -half_size);
            glVertex3f(-half_size, half_size, -half_size);
            glVertex3f(half_size, half_size, -half_size);
            glVertex3f(half_size, -half_size, -half_size);
            
            // Top face
            glVertex3f(-half_size, half_size, -half_size);
            glVertex3f(-half_size, half_size, half_size);
            glVertex3f(half_size, half_size, half_size);
            glVertex3f(half_size, half_size, -half_size);
            
            // Bottom face
            glVertex3f(-half_size, -half_size, -half_size);
            glVertex3f(half_size, -half_size, -half_size);
            glVertex3f(half_size, -half_size, half_size);
            glVertex3f(-half_size, -half_size, half_size);
            
            // Left face
            glVertex3f(-half_size, -half_size, -half_size);
            glVertex3f(-half_size, -half_size, half_size);
            glVertex3f(-half_size, half_size, half_size);
            glVertex3f(-half_size, half_size, -half_size);
            
            // Right face
            glVertex3f(half_size, -half_size, -half_size);
            glVertex3f(half_size, half_size, -half_size);
            glVertex3f(half_size, half_size, half_size);
            glVertex3f(half_size, -half_size, half_size);
            
            glEnd();
            
            glPopMatrix();
        }
    }
    
    glPopAttrib();
}

// Clean up all particle effects
void cleanupParticleSystem(void) {
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        if (effects[i].particles) {
            free(effects[i].particles);
            effects[i].particles = NULL;
        }
        effects[i].active = false;
    }
}