#include <SDL2/SDL.h>
#include "../include/input.h"
#include "../include/player.h"
#include "../include/game.h"
#include "../include/log.h"

// Initialize the input system
void initInput(InputState* input) {
    // Clear all key states
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        input->keys[i] = false;
    }
    
    // Initialize mouse state
    input->mouse_x = 0;
    input->mouse_y = 0;
    input->mouse_dx = 0;
    input->mouse_dy = 0;
    
    for (int i = 0; i < 5; i++) {
        input->mouse_buttons[i] = false;
    }
    
    input->mouse_locked = false;
    input->mouse_sensitivity = 0.1f;
}

// Process input events and update player controls
void processInput(InputState* input, GameState* game, Player* player, float delta_time) {
    // If in menu mode, don't process movement inputs
    if (game->menu_state != MENU_NONE) {
        return;
    }
    
    // Update camera based on mouse movement if mouse is locked
    if (input->mouse_locked) {
        player->yaw += input->mouse_dx * input->mouse_sensitivity;
        
        // Apply Y-axis inversion if enabled in settings
        float pitchFactor = game->settings.invert_y_axis ? 1.0f : -1.0f;
        player->pitch += pitchFactor * input->mouse_dy * input->mouse_sensitivity;
        
        // Normalize yaw to 0-360 degrees
        if (player->yaw >= 360.0f) player->yaw -= 360.0f;
        if (player->yaw < 0.0f) player->yaw += 360.0f;
    }
    
    // Reset deltas after processing
    input->mouse_dx = 0;
    input->mouse_dy = 0;
    
    // Process key inputs for movement (if game has started)
    if (game->game_started) {
        // Forward/backward movement - W/S or Up/Down arrows
        if (input->keys[SDL_SCANCODE_W] || input->keys[SDL_SCANCODE_UP]) {
            movePlayerForward(player, delta_time);
        }
        if (input->keys[SDL_SCANCODE_S] || input->keys[SDL_SCANCODE_DOWN]) {
            movePlayerBackward(player, delta_time);
        }
        
        // Strafe left/right - A/D or Left/Right arrows
        if (input->keys[SDL_SCANCODE_A] || input->keys[SDL_SCANCODE_LEFT]) {
            movePlayerLeft(player, delta_time);
        }
        if (input->keys[SDL_SCANCODE_D] || input->keys[SDL_SCANCODE_RIGHT]) {
            movePlayerRight(player, delta_time);
        }
        
        // Jump with space
        if (input->keys[SDL_SCANCODE_SPACE]) {
            playerJump(player);
        }
    }
}

// Handle key down events
void handleKeyDown(InputState* input, SDL_Keycode key, GameState* game) {
    // Handle F11 for fullscreen toggle in all application states
    if (key == SDLK_F11) {
        // Toggle fullscreen using our unified function
        toggleFullscreen(game, !game->fullscreen);
        return; // Handle F11 and return immediately
    }
    
    // If we're in a menu, handle menu navigation
    if (game->menu_state != MENU_NONE) {
        handleMenuInput(game, key);
        return;
    }
    
    // Convert SDL_Keycode to SDL_Scancode for tracking in keys array
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    input->keys[scancode] = true;
    
    // Handle special keys that aren't captured in the keys array
    switch (key) {
        case SDLK_ESCAPE:
            if (game->menu_state == MENU_MAIN) {
                game->running = false; // Set running to false instead of quit=true
            } else {
                // Show menu and pause game
                game->menu_state = MENU_MAIN;
                game->game_paused = true;
                game->selected_menu_item = 0; // Select RESUME GAME by default
                updateMenuUI(game);
                
                // Release mouse capture
                if (input->mouse_locked) {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    input->mouse_locked = false;
                }
            }
            break;
            
        case SDLK_SPACE:
            input->keys[SDL_SCANCODE_SPACE] = true;
            break;
        case SDLK_LEFT:
            input->keys[SDL_SCANCODE_LEFT] = true;
            break;
        case SDLK_RIGHT:
            input->keys[SDL_SCANCODE_RIGHT] = true;
            break;
        case SDLK_UP:
            input->keys[SDL_SCANCODE_UP] = true;
            break;
        case SDLK_DOWN:
            input->keys[SDL_SCANCODE_DOWN] = true;
            break;
    }
}

// Handle key up events
void handleKeyUp(InputState* input, SDL_Keycode key) {
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    input->keys[scancode] = false;
}

// Handle mouse motion events
void handleMouseMotion(InputState* input, int x, int y, int dx, int dy) {
    input->mouse_x = x;
    input->mouse_y = y;
    
    if (input->mouse_locked) {
        // Accumulate delta movement for smooth camera control
        input->mouse_dx += dx;
        input->mouse_dy += dy;
    }
}

// Handle mouse button events
void handleMouseButton(InputState* input, int button, bool state) {
    if (button >= 0 && button < 5) {
        input->mouse_buttons[button] = state;
    }
    
    // Left click (button 0) in game mode enables mouse lock for camera control if not locked
    if (button == 0 && state == true) {
        if (!input->mouse_locked) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            input->mouse_locked = true;
        } else {
            // If already locked (in-game), left click triggers foliage cutting
            // The cutMediumFoliage function will be called in updateGame when player.is_cutting is true
            extern GameState *game_state_ptr; // Use global pointer to current game state
            startCuttingFoliage(&game_state_ptr->player);
        }
    }
}

// Center the mouse in the window
void centerMouseInWindow(SDL_Window* window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_WarpMouseInWindow(window, w/2, h/2);
}