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

// Spawn foliage cutting particles at the given coordinates using the provided texture
void spawnFoliageParticles(float x, float y, float z, GLuint texture) {
    // Find an inactive effect slot
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        if (!effects[i].active) {
            // Initialize the effect
            effects[i].active = true;
            effects[i].timer = 0.0f;
            effects[i].x = x;
            effects[i].y = y;
            effects[i].z = z;
            effects[i].count = FOLIAGE_PARTICLES;
            
            // Allocate memory for particles
            effects[i].particles = (Particle*)malloc(effects[i].count * sizeof(Particle));
            
            // Initialize each particle with a random section of the texture
            for (int j = 0; j < effects[i].count; j++) {
                Particle* p = &effects[i].particles[j];
                p->active = true;
                p->stage = PARTICLE_FOLIAGE;
                
                // Random position near the cut point
                p->x = x + ((float)rand() / RAND_MAX * 0.6f - 0.3f);
                p->y = y + ((float)rand() / RAND_MAX * 0.6f);  // More upward bias
                p->z = z + ((float)rand() / RAND_MAX * 0.6f - 0.3f);
                
                // Random velocity - mostly upward and outward
                p->vx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 1.5f;
                p->vy = ((float)rand() / RAND_MAX * 1.5f + 0.5f) * 2.0f; // Upward bias
                p->vz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 1.5f;
                
                // Random size for the particle
                p->initial_size = 0.15f + (float)rand() / RAND_MAX * 0.2f;
                p->size = p->initial_size;
                
                // Give it the texture from the foliage
                p->texture = texture;
                
                // Random UV coordinates - select a small section of the texture
                p->u1 = (float)rand() / RAND_MAX * 0.7f;
                p->v1 = (float)rand() / RAND_MAX * 0.7f;
                
                // Make sure the texture section isn't too small
                p->u2 = p->u1 + 0.2f + (float)rand() / RAND_MAX * 0.1f;
                p->v2 = p->v1 + 0.2f + (float)rand() / RAND_MAX * 0.1f;
                
                // Clamp UV coordinates to valid range
                if (p->u2 > 1.0f) p->u2 = 1.0f;
                if (p->v2 > 1.0f) p->v2 = 1.0f;
                
                // Random initial rotation
                p->rotation = (float)rand() / RAND_MAX * 360.0f;
                
                // Set lifetime
                p->max_life = PARTICLE_FOLIAGE_DURATION;
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
            } else if (p->stage != PARTICLE_FOLIAGE) {
                // Only apply stage transitions for explosion particles
                if (p->life <= p->max_life - PARTICLE_RED_DURATION - PARTICLE_WHITE_DURATION) {
                    p->stage = PARTICLE_GRAY;
                } else if (p->life <= p->max_life - PARTICLE_RED_DURATION) {
                    p->stage = PARTICLE_WHITE;
                }
            }
            
            // Update position based on velocity
            p->x += p->vx * delta;
            p->y += p->vy * delta;
            p->z += p->vz * delta;
            
            // Add gravity effect
            p->vy -= 2.0f * delta;
            
            // Scale size based on lifetime and particle type
            if (p->stage == PARTICLE_FOLIAGE) {
                // Foliage particles maintain their size but may rotate
                // We could add some slight size variation here if desired
                float life_percent = p->life / p->max_life;
                // Slow down as they fall
                p->vx *= 0.97f;
                p->vz *= 0.97f;
                
                // Add some gentle spinning effect
                p->rotation += delta * (30.0f + ((float)rand() / RAND_MAX) * 20.0f);
            } else if (p->stage == PARTICLE_RED) {
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
    // Save current state
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    
    for (int i = 0; i < MAX_PARTICLE_EFFECTS; i++) {
        if (!effects[i].active) continue;
        
        for (int j = 0; j < effects[i].count; j++) {
            Particle* p = &effects[i].particles[j];
            if (!p->active) continue;
            
            if (p->stage == PARTICLE_FOLIAGE) {
                // Special rendering for textured foliage particles
                
                // Enable texturing and blending for foliage particles
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                // Bind the texture from the foliage
                glBindTexture(GL_TEXTURE_2D, p->texture);
                
                // Calculate the remaining life percentage for fade out
                float life_percent = p->life / p->max_life;
                
                // Full white color with alpha based on remaining life
                glColor4f(1.0f, 1.0f, 1.0f, life_percent);
                
                glPushMatrix();
                // Position the particle
                glTranslatef(p->x, p->y, p->z);
                
                // Get the modelview matrix to create billboards facing camera
                float modelview[16];
                glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
                
                // Extract the camera right and up vectors
                float right_x = modelview[0];
                float right_y = modelview[4];
                float right_z = modelview[8];
                float up_x = modelview[1];
                float up_y = modelview[5];
                float up_z = modelview[9];
                
                // Apply rotation around the view vector
                float angle = p->rotation + p->life * 60.0f; // Rotate over time
                float rad = angle * M_PI / 180.0f;
                float cos_a = cosf(rad);
                float sin_a = sinf(rad);
                
                float r_x = right_x * cos_a + up_x * sin_a;
                float r_y = right_y * cos_a + up_y * sin_a;
                float r_z = right_z * cos_a + up_z * sin_a;
                float u_x = -right_x * sin_a + up_x * cos_a;
                float u_y = -right_y * sin_a + up_y * cos_a;
                float u_z = -right_z * sin_a + up_z * cos_a;
                
                float half_size = p->size / 2.0f;
                
                // Draw a textured quad for the foliage piece
                glBegin(GL_QUADS);
                // Bottom left
                glTexCoord2f(p->u1, p->v2);
                glVertex3f(-half_size * r_x - half_size * u_x, 
                           -half_size * r_y - half_size * u_y, 
                           -half_size * r_z - half_size * u_z);
                
                // Bottom right
                glTexCoord2f(p->u2, p->v2);
                glVertex3f(half_size * r_x - half_size * u_x, 
                           half_size * r_y - half_size * u_y, 
                           half_size * r_z - half_size * u_z);
                
                // Top right
                glTexCoord2f(p->u2, p->v1);
                glVertex3f(half_size * r_x + half_size * u_x, 
                           half_size * r_y + half_size * u_y, 
                           half_size * r_z + half_size * u_z);
                
                // Top left
                glTexCoord2f(p->u1, p->v1);
                glVertex3f(-half_size * r_x + half_size * u_x, 
                           -half_size * r_y + half_size * u_y, 
                           -half_size * r_z + half_size * u_z);
                glEnd();
                
                glPopMatrix();
                glDisable(GL_TEXTURE_2D);
            } else {
                // Regular particle rendering (explosions, etc.)
                glDisable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
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