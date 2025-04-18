#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../include/player.h"
#include "../include/config.h"

// Initialize the player with default values and load weapon model
void initPlayer(Player *player) {
    // Initialize position and orientation
    player->position_x = 0.0f;
    player->position_z = 0.0f;
    player->position_y = 0.0f;  // Will be set properly based on ground level
    player->velocity_x = 0.0f;
    player->velocity_y = 0.0f;
    player->velocity_z = 0.0f;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    
    // Physics properties
    player->height = 1.8f;
    player->eye_height = 1.7f; // Eyes are slightly below the top of player
    player->ground_level = 0.0f; // Will be updated based on terrain
    player->movement_speed = 5.0f; // Units per second
    player->jump_velocity = 5.0f;
    player->gravity = 9.8f;
    
    // State flags
    player->is_moving = false;
    player->is_jumping = false;
    
    // Load weapon model (if available)
    player->weapon_model = loadModel(PLAYER_WEAPON_MODEL);
    if (player->weapon_model.num_meshes == 0) {
        printf("Warning: Failed to load weapon model!\n");
        return;
    }
    
    // Allocate and generate buffers for the weapon model
    player->weapon_vbos = (GLuint*)malloc(player->weapon_model.num_meshes * sizeof(GLuint));
    player->weapon_ibos = (GLuint*)malloc(player->weapon_model.num_meshes * sizeof(GLuint));
    
    glGenBuffers(player->weapon_model.num_meshes, player->weapon_vbos);
    glGenBuffers(player->weapon_model.num_meshes, player->weapon_ibos);
    
    for (unsigned int i = 0; i < player->weapon_model.num_meshes; i++) {
        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, player->weapon_vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, player->weapon_model.num_vertices[i] * 12 * sizeof(float),
                    player->weapon_model.vertices[i], GL_STATIC_DRAW);
                    
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player->weapon_ibos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, player->weapon_model.num_indices[i] * sizeof(unsigned int),
                    player->weapon_model.indices[i], GL_STATIC_DRAW);
    }
}

// Update player state based on time
void updatePlayer(Player *player, float delta_time) {
    // Apply gravity if not on ground
    if (player->position_y > player->ground_level + player->height * 0.5f) {
        player->velocity_y -= player->gravity * delta_time;
    } else {
        // On ground, stop falling
        player->position_y = player->ground_level + player->height * 0.5f;
        player->velocity_y = 0;
        player->is_jumping = false;
    }
    
    // Apply velocity to position
    player->position_x += player->velocity_x * delta_time;
    player->position_y += player->velocity_y * delta_time;
    player->position_z += player->velocity_z * delta_time;
    
    // Dampen horizontal velocity (friction)
    player->velocity_x *= 0.9f;
    player->velocity_z *= 0.9f;
    
    // Clamp pitch to avoid looking too far up or down
    if (player->pitch > 89.0f) player->pitch = 89.0f;
    if (player->pitch < -89.0f) player->pitch = -89.0f;
}

// Render the player's weapon
void renderWeapon(Player *player) {
    if (player->weapon_model.num_meshes == 0) return;
    
    glPushMatrix();
    
    // Position weapon in front of camera
    glTranslatef(0.3f, -0.3f, -0.7f);
    glRotatef(10.0f, 0.0f, 1.0f, 0.0f); // Slight angle adjustment
    glScalef(0.2f, 0.2f, 0.2f); // Scale down the weapon
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // Render each mesh of the weapon
    for (unsigned int i = 0; i < player->weapon_model.num_meshes; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, player->weapon_vbos[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player->weapon_ibos[i]);
        
        // Set up vertex pointers
        glVertexPointer(3, GL_FLOAT, 12 * sizeof(float), (void*)0);
        glNormalPointer(GL_FLOAT, 12 * sizeof(float), (void*)(3 * sizeof(float)));
        glTexCoordPointer(2, GL_FLOAT, 12 * sizeof(float), (void*)(6 * sizeof(float)));
        glColorPointer(4, GL_FLOAT, 12 * sizeof(float), (void*)(8 * sizeof(float)));
        
        // Bind texture
        glBindTexture(GL_TEXTURE_2D, player->weapon_model.texture_ids[i]);
        
        // Draw mesh
        glDrawElements(GL_TRIANGLES, player->weapon_model.num_indices[i], GL_UNSIGNED_INT, 0);
    }
    
    // Disable vertex attributes
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glPopMatrix();
}

// Clean up player resources
void cleanupPlayer(Player *player) {
    // Cleanup weapon model
    if (player->weapon_vbos) {
        glDeleteBuffers(player->weapon_model.num_meshes, player->weapon_vbos);
        free(player->weapon_vbos);
        player->weapon_vbos = NULL;
    }
    
    if (player->weapon_ibos) {
        glDeleteBuffers(player->weapon_model.num_meshes, player->weapon_ibos);
        free(player->weapon_ibos);
        player->weapon_ibos = NULL;
    }
    
    freeModel(player->weapon_model);
}

// Movement functions
void movePlayerForward(Player *player, float delta_time) {
    float moveX = sinf(player->yaw * M_PI / 180.0f);
    float moveZ = cosf(player->yaw * M_PI / 180.0f);
    
    player->velocity_x = moveX * player->movement_speed;
    player->velocity_z = -moveZ * player->movement_speed; // Negative for OpenGL coordinate system
    
    player->is_moving = true;
}

void movePlayerBackward(Player *player, float delta_time) {
    float moveX = sinf(player->yaw * M_PI / 180.0f);
    float moveZ = cosf(player->yaw * M_PI / 180.0f);
    
    player->velocity_x = -moveX * player->movement_speed;
    player->velocity_z = moveZ * player->movement_speed; // Positive for OpenGL coordinate system
    
    player->is_moving = true;
}

void movePlayerLeft(Player *player, float delta_time) {
    float moveX = sinf((player->yaw - 90.0f) * M_PI / 180.0f);
    float moveZ = cosf((player->yaw - 90.0f) * M_PI / 180.0f);
    
    player->velocity_x = moveX * player->movement_speed;
    player->velocity_z = -moveZ * player->movement_speed; // Negative for OpenGL coordinate system
    
    player->is_moving = true;
}

void movePlayerRight(Player *player, float delta_time) {
    float moveX = sinf((player->yaw + 90.0f) * M_PI / 180.0f);
    float moveZ = cosf((player->yaw + 90.0f) * M_PI / 180.0f);
    
    player->velocity_x = moveX * player->movement_speed;
    player->velocity_z = -moveZ * player->movement_speed; // Negative for OpenGL coordinate system
    
    player->is_moving = true;
}

void playerJump(Player *player) {
    // Only allow jumping when on the ground
    if (!player->is_jumping) {
        player->velocity_y = player->jump_velocity;
        player->is_jumping = true;
    }
}