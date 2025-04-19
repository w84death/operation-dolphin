#include <SDL2/SDL.h>
#include "../include/input.h"
#include "../include/player.h"
#include "../include/game.h"

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
        // Forward/backward movement
        if (input->keys[SDL_SCANCODE_W]) {
            movePlayerForward(player, delta_time);
        }
        if (input->keys[SDL_SCANCODE_S]) {
            movePlayerBackward(player, delta_time);
        }
        
        // Strafe left/right
        if (input->keys[SDL_SCANCODE_A]) {
            movePlayerLeft(player, delta_time);
        }
        if (input->keys[SDL_SCANCODE_D]) {
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
            break;
            
        case SDLK_F11:
            // Toggle fullscreen
            game->fullscreen = !game->fullscreen;
            
            if (game->fullscreen) {
                // Store the current window size before going fullscreen
                SDL_GetWindowSize(game->window, &game->window_width, &game->window_height);
                
                // For fullscreen, use SDL_WINDOW_FULLSCREEN_DESKTOP for better compatibility
                if (SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                    printf("Error switching to fullscreen: %s\n", SDL_GetError());
                }
                
                // Get the new resolution after switching to fullscreen
                int new_width, new_height;
                SDL_GetWindowSize(game->window, &new_width, &new_height);
                
                // Update UI positions based on new resolution
                repositionUI(&game->game_ui, new_width, new_height);
                repositionUI(&game->menu_ui, new_width, new_height);
                
                // Update OpenGL viewport
                glViewport(0, 0, new_width, new_height);
                
                // Update projection matrix for the new aspect ratio
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                float aspect = (float)new_width / (float)new_height;
                float fov = CAMERA_FOV;
                float near = CAMERA_NEAR;
                float far = CAMERA_FAR;
                float fH = tan(fov * 3.14159f / 360.0f) * near;
                float fW = fH * aspect;
                glFrustum(-fW, fW, -fH, fH, near, far);
                glMatrixMode(GL_MODELVIEW);
                
                printf("Switched to fullscreen: %dx%d\n", new_width, new_height);
            } else {
                // Return to windowed mode with original dimensions
                if (SDL_SetWindowFullscreen(game->window, 0) != 0) {
                    printf("Error switching to windowed mode: %s\n", SDL_GetError());
                }
                
                // Restore original window size
                SDL_SetWindowSize(game->window, WINDOW_WIDTH, WINDOW_HEIGHT);
                
                // Update UI positions based on original resolution
                repositionUI(&game->game_ui, WINDOW_WIDTH, WINDOW_HEIGHT);
                repositionUI(&game->menu_ui, WINDOW_WIDTH, WINDOW_HEIGHT);
                
                // Update OpenGL viewport
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
                
                // Update projection matrix for the original aspect ratio
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
                float fov = CAMERA_FOV;
                float near = CAMERA_NEAR;
                float far = CAMERA_FAR;
                float fH = tan(fov * 3.14159f / 360.0f) * near;
                float fW = fH * aspect;
                glFrustum(-fW, fW, -fH, fH, near, far);
                glMatrixMode(GL_MODELVIEW);
                
                printf("Switched to windowed mode: %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
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