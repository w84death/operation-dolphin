#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "player.h"
#include "ui.h"
#include "config.h"
#include "audio.h"
#include "vegetation.h"

// Settings file path
#define SETTINGS_FILE_PATH "user_settings.cfg"

// Menu states
typedef enum {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_NONE,  // Game is running, no menu shown
    MENU_PHOTO_MODE  // New photo mode state
} MenuState;

// Settings options
typedef struct {
    bool sound_enabled;
    bool high_terrain_features;
    bool invert_y_axis;  // Y-axis inversion setting
    bool fullscreen;     // Fullscreen toggle setting
    unsigned int foliage_seed; // Seed for foliage generation
} GameSettings;

// Game state structure
typedef struct {
    bool running;
    bool fullscreen;
    bool game_started;
    bool game_paused;
    float last_time;
    
    // Screen resolution tracking
    int window_width;
    int window_height;
    
    // FPS tracking
    int frame_count;
    Uint32 fps_last_time;
    
    // Player, camera and world
    Player player;
    void* terrain; // Opaque pointer to terrain data
    
    // SDL/OpenGL resources
    SDL_Window* window;
    SDL_GLContext gl_context;
    
    // UI Systems - separate UI systems for game and menu
    UISystem game_ui;    // Smaller font for in-game UI
    UISystem menu_ui;    // Larger font for menu UI
    
    int fps_text_id;
    
    // Compass UI elements
    int compass_n_id;
    int compass_e_id;
    int compass_s_id;
    int compass_w_id;
    int compass_indicator_id;
    
    // Menu system
    MenuState menu_state;
    int selected_menu_item;
    int menu_item_count;
    GameSettings settings;
    
    // Menu UI elements
    int menu_title_id;
    int menu_version_id;  // New field for engine version text
    int menu_items[6];
    int settings_items[4]; // Increased from 3 to 4 to accommodate fullscreen setting
    int settings_values[4]; // Increased from 3 to 4 to accommodate fullscreen setting

    // Audio system
    AudioSystem audio;
} GameState;

// Function declarations
bool initGame(GameState* game);
void gameLoop(GameState* game);
void handleEvents(GameState* game);
void updateGame(GameState* game, float delta_time);
void renderGame(GameState* game);
void cleanupGame(GameState* game);
void updateGameStats(GameState* game);
void initMenu(GameState* game);
void updateMenuUI(GameState* game);
void handleMenuInput(GameState* game, SDL_Keycode key);
void resetGame(GameState* game);
void updateCompassUI(GameState* game); // Added missing declaration
void cutMediumFoliage(Player* player); // New function to handle cutting medium foliage
bool saveSettings(GameSettings* settings); // New function to save settings
bool loadSettings(GameSettings* settings); // New function to load settings
void toggleFullscreen(GameState* game, bool fullscreen); // Function to consistently toggle fullscreen mode

#endif // GAME_H