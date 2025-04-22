#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <GL/gl.h>
#include "model.h"
#include "config.h"
#include "audio.h"  // Include audio for sound effects

// Forward declaration for terrain
typedef struct Terrain Terrain;
typedef struct AudioSystem AudioSystem;

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
    
    // Terrain reference for height checks
    void* terrain;     // Pointer to terrain for height checks
    
    // Audio system reference for sound effects
    AudioSystem* audio;
    
    // Weapon/interaction
    GLuint weapon_texture_id;  // Texture ID for 2D weapon instead of 3D model
    int weapon_frame;          // Current animation frame (0, 1, or 2)
    float weapon_anim_timer;   // Timer for weapon animation
    bool is_cutting;           // Whether the player is currently cutting foliage
    float cutting_cooldown;    // Cooldown timer between cuts
} Player;

// Function declarations
void initPlayer(Player *player, AudioSystem* audio);
void updatePlayer(Player *player, float delta_time);
void renderWeapon(Player *player);
void cleanupPlayer(Player *player);

// Movement functions
void movePlayerForward(Player *player, float delta_time);
void movePlayerBackward(Player *player, float delta_time);
void movePlayerLeft(Player *player, float delta_time);
void movePlayerRight(Player *player, float delta_time);
void playerJump(Player *player);

// Weapon and foliage interaction
void startCuttingFoliage(Player *player);
void updateWeaponAnimation(Player *player, float delta_time);
void loadWeaponTexture(Player *player, const char* texture_path);

#endif // PLAYER_H