#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../include/player.h"
#include "../include/terrain.h" // Add terrain header for height checks
#include "../include/config.h"
#include "../include/log.h"
#include "../stb_image.h" // Include stb_image for texture loading

// Load weapon texture from file
void loadWeaponTexture(Player *player, const char* texture_path) {
    log_info("Loading weapon texture from: %s", texture_path);
    
    int width, height, channels;
    unsigned char *image = stbi_load(texture_path, &width, &height, &channels, STBI_rgb_alpha);
    
    if (!image) {
        log_error("Error loading weapon texture: %s", stbi_failure_reason());
        return;
    }
    
    // Delete previous texture if it exists
    if (player->weapon_texture_id != 0) {
        glDeleteTextures(1, &player->weapon_texture_id);
    }
    
    // Generate texture
    glGenTextures(1, &player->weapon_texture_id);
    glBindTexture(GL_TEXTURE_2D, player->weapon_texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // Free the image data as it's now in GPU memory
    stbi_image_free(image);
    
    log_success("Weapon texture loaded successfully, ID: %u", player->weapon_texture_id);
}

// Update weapon animation state
void updateWeaponAnimation(Player *player, float delta_time) {
    // Handle cutting cooldown
    if (player->cutting_cooldown > 0.0f) {
        player->cutting_cooldown -= delta_time;
        if (player->cutting_cooldown <= 0.0f) {
            player->cutting_cooldown = 0.0f;
        }
    }
    
    // Only update animation if currently cutting
    if (player->is_cutting) {
        player->weapon_anim_timer += delta_time;
        
        // Update animation frames
        if (player->weapon_anim_timer >= WEAPON_ANIM_FRAME_TIME) {
            player->weapon_frame = (player->weapon_frame + 1) % 3; // Cycle through 0, 1, 2
            player->weapon_anim_timer = 0.0f;
            
            // Load the appropriate texture for this frame
            const char* texture_path;
            switch (player->weapon_frame) {
                case 0:
                    texture_path = PLAYER_WEAPON_TEXTURE_0;
                    break;
                case 1:
                    texture_path = PLAYER_WEAPON_TEXTURE_1;
                    break;
                case 2:
                    texture_path = PLAYER_WEAPON_TEXTURE_2;
                    break;
            }
            loadWeaponTexture(player, texture_path);
            
            // If we've completed the animation cycle, stop cutting
            if (player->weapon_frame == 0) {
                player->is_cutting = false;
            }
        }
    }
}

// Start the foliage cutting action
void startCuttingFoliage(Player *player) {
    // Only start cutting if not already cutting and cooldown has expired
    if (!player->is_cutting && player->cutting_cooldown <= 0.0f) {
        player->is_cutting = true;
        player->weapon_frame = 0;
        player->weapon_anim_timer = 0.0f;
        player->cutting_cooldown = FOLIAGE_CUTTING_COOLDOWN;
        
        // Load the first frame of the cutting animation
        loadWeaponTexture(player, PLAYER_WEAPON_TEXTURE_0);
        
        // Log that we're cutting
        log_info("Player is cutting foliage");
    }
}

// Initialize the player with default values and load weapon texture
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
    
    // Physics properties - use values from config.h
    player->height = 2.0f;
    player->eye_height = PLAYER_EYE_HEIGHT;
    player->ground_level = 0.0f; // Will be updated based on terrain
    player->movement_speed = PLAYER_MOVEMENT_SPEED;
    player->jump_velocity = PLAYER_JUMP_VELOCITY;
    player->gravity = PLAYER_GRAVITY;
    
    // State flags
    player->is_moving = false;
    player->is_jumping = false;
    
    // Weapon animation and interaction properties
    player->weapon_frame = 0;
    player->weapon_anim_timer = 0.0f;
    player->is_cutting = false;
    player->cutting_cooldown = 0.0f;
    
    // Load initial weapon texture
    loadWeaponTexture(player, PLAYER_WEAPON_TEXTURE_0);
}

// Update player state based on time
void updatePlayer(Player *player, float delta_time) {
    // If we have a valid terrain reference, get the current ground height at player position
    if (player->terrain) {
        // Get the terrain height at the player's current position
        float terrain_height = getHeightAtPoint((Terrain*)player->terrain, player->position_x, player->position_z);
        
        // Update the player's ground level based on the terrain
        player->ground_level = terrain_height;
    }
    
    // Apply gravity if not on ground
    if (player->position_y > player->ground_level + player->height * 0.5f) {
        player->velocity_y -= player->gravity * delta_time;
    } else {
        // On ground, stop falling and match the terrain height
        player->position_y = player->ground_level + player->height * 0.5f;
        player->velocity_y = 0;
        player->is_jumping = false;
    }
    
    // Store current position to check if the player moved
    float prev_x = player->position_x;
    float prev_z = player->position_z;
    
    // Apply velocity to position
    player->position_x += player->velocity_x * delta_time;
    player->position_y += player->velocity_y * delta_time;
    player->position_z += player->velocity_z * delta_time;
    
    // If the player moved horizontally and is on the ground, adjust height to follow terrain
    if ((prev_x != player->position_x || prev_z != player->position_z) && 
        player->terrain && !player->is_jumping) {
        
        // Get new terrain height at updated position
        float new_terrain_height = getHeightAtPoint((Terrain*)player->terrain, 
                                                  player->position_x, player->position_z);
        
        // Calculate height difference
        float height_diff = new_terrain_height - player->ground_level;
        
        // If the slope is too steep (more than 45 degrees), limit movement
        const float MAX_SLOPE_CHANGE = 0.75f;  // Maximum height change per movement step
        
        if (fabsf(height_diff) > MAX_SLOPE_CHANGE) {
            // Slope is too steep, reduce horizontal movement
            float scale = MAX_SLOPE_CHANGE / fabsf(height_diff);
            
            // Scale back the position change
            player->position_x = prev_x + (player->position_x - prev_x) * scale;
            player->position_z = prev_z + (player->position_z - prev_z) * scale;
            
            // Recalculate terrain height at the adjusted position
            new_terrain_height = getHeightAtPoint((Terrain*)player->terrain, 
                                               player->position_x, player->position_z);
        }
        
        // Update ground level with the new terrain height
        player->ground_level = new_terrain_height;
        
        // If on ground, adjust player's y position to match terrain
        if (!player->is_jumping) {
            player->position_y = player->ground_level + player->height * 0.5f;
        }
    }
    
    // Dampen horizontal velocity (friction)
    player->velocity_x *= 0.9f;
    player->velocity_z *= 0.9f;
    
    // Clamp pitch to respect PLAYER_MAXIMUM_VERTICAL_ROT from config.h
    float half_angle = PLAYER_MAXIMUM_VERTICAL_ROT / 2.0f;
    if (player->pitch > half_angle) player->pitch = half_angle;
    if (player->pitch < -half_angle) player->pitch = -half_angle;
    
    // Update weapon animation
    updateWeaponAnimation(player, delta_time);
}

// Render the player's weapon
void renderWeapon(Player *player) {
    // Check if we have a valid texture
    if (player->weapon_texture_id == 0) return;
    
    // Save current matrix and attributes
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Set up proper blending for the texture
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Make sure we're in texture 2D mode
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, player->weapon_texture_id);
    
    // Position the weapon texture in view
    glTranslatef(0.5f, -0.4f, -1.0f);
    
    // Apply a slight rotation to match the player's view
    glRotatef(-5.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(5.0f, 0.0f, 1.0f, 0.0f);
    
    // Draw a textured quad for the weapon
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, 0.0f);  // Bottom left
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, -0.5f, 0.0f);   // Bottom right
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.0f);    // Top right
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.0f);   // Top left
    glEnd();
    
    // Restore previous state
    glPopAttrib();
    glPopMatrix();
}

// Clean up player resources
void cleanupPlayer(Player *player) {
    // Delete the weapon texture if it exists
    if (player->weapon_texture_id != 0) {
        glDeleteTextures(1, &player->weapon_texture_id);
        player->weapon_texture_id = 0;
        log_info("Weapon texture released");
    }
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