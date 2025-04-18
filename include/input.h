#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "player.h"
#include "game.h"

// Input state structure
typedef struct {
    bool keys[SDL_NUM_SCANCODES];
    
    // Mouse state
    int mouse_x, mouse_y;
    int mouse_dx, mouse_dy; // Delta/change in mouse position
    bool mouse_buttons[5];  // Left, middle, right, and extra buttons
    bool mouse_locked;      // Is mouse locked to window center
    
    // Sensitivity settings
    float mouse_sensitivity;
} InputState;

// Function declarations
void initInput(InputState* input);
void processInput(InputState* input, GameState* game, Player* player, float delta_time);
void handleKeyDown(InputState* input, SDL_Keycode key, GameState* game);
void handleKeyUp(InputState* input, SDL_Keycode key);
void handleMouseMotion(InputState* input, int x, int y, int dx, int dy);
void handleMouseButton(InputState* input, int button, bool state);
void centerMouseInWindow(SDL_Window* window);

#endif // INPUT_H