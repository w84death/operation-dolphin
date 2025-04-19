// OPERATION DOLPHIN
// Description: Main entry point for the FPS game
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include "include/game.h"
#include "include/log.h"
#include "include/config.h"

int main() {
    // Display welcome message with game title
    display_title("OPERATION DOLPHIN: ECHOES OF THE MESOZOIC");
    log_info("Game version: Alpha1");
    log_info("© 2025 P1X GAMES - All Rights Reserved");
    log_info("Starting game initialization...");
    
    // Create game state
    GameState game;
    
    // Initialize game
    if (!initGame(&game)) {
        log_error("Failed to initialize game!");
        return 1;
    }
    
    // Run the game loop
    gameLoop(&game);
    
    // Clean up resources
    cleanupGame(&game);
    
    return 0;
}
