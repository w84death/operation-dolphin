#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "player.h"
#include "terrain.h"
#include "config.h"
#include "ui.h"

// Forward declaration for terrain type
typedef struct Terrain Terrain;

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
} GameSettings;

// Audio structure for managing game audio
typedef struct {
    Mix_Music **music_tracks;        // Array of music tracks
    Mix_Music *menu_music;           // Special music track for menu screens
    char **music_filenames;          // Array of music filenames
    int num_tracks;                  // Total number of music tracks
    int current_track;               // Current playing track index
    int *gameplay_playlist;          // Array of indices for gameplay music
    int num_gameplay_tracks;         // Number of tracks in gameplay playlist
    int current_gameplay_track;      // Current gameplay track index
    bool music_playing;              // Whether music is currently playing
    bool initialized;                // Whether audio system is initialized
    bool in_menu_music;              // Whether currently playing menu music
    float track_switch_timer;        // Timer for track switching
} AudioSystem;

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
    Terrain* terrain;
    
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
void cutMediumFoliage(Player* player); // New function to handle cutting medium foliage

#endif // GAME_H