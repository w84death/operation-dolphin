// OPERAION DOLPHIN
// Description: Main entry point for the FPS game
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include "include/game.h"

int main() {
    // Create game state
    GameState game;
    
    // Initialize game
    if (!initGame(&game)) {
        printf("Failed to initialize game!\n");
        return 1;
    }
    
    // Run the game loop
    gameLoop(&game);
    
    // Clean up resources
    cleanupGame(&game);
    
    return 0;
}
