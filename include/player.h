#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <GL/gl.h>
#include "model.h"
#include "config.h"

// Player state structure
typedef struct {
    // Position and orientation
    float position_x;
    float position_y;
    float position_z;
    float velocity_x;
    float velocity_y;
    float velocity_z;
    float yaw;   // Horizontal rotation (left/right)
    float pitch; // Vertical rotation (up/down)
    
    // Player state
    bool is_moving;
    bool is_jumping;
    float height;
    float eye_height;
    float ground_level;
    float movement_speed;
    float jump_velocity;
    float gravity;
    
    // Weapon/interaction
    GLuint weapon_texture_id;  // Texture ID for 2D weapon instead of 3D model
} Player;

// Function declarations
void initPlayer(Player *player);
void updatePlayer(Player *player, float delta_time);
void renderWeapon(Player *player);
void cleanupPlayer(Player *player);

// Movement functions
void movePlayerForward(Player *player, float delta_time);
void movePlayerBackward(Player *player, float delta_time);
void movePlayerLeft(Player *player, float delta_time);
void movePlayerRight(Player *player, float delta_time);
void playerJump(Player *player);

#endif // PLAYER_H