#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "ui.h"

// Menu states
typedef enum {
    MENU_NONE,
    MENU_MAIN,
    MENU_SETTINGS
} MenuState;

// Game settings structure
typedef struct {
    bool sound_enabled;
    bool high_terrain_features;
    bool invert_y_axis;
    bool fullscreen;
} GameSettings;

// Forward declaration of AudioSystem
struct AudioSystem;

// Menu system structure
typedef struct {
    MenuState state;
    int selected_item;
    int item_count;
    bool game_paused;
    bool game_started;
    
    // UI elements for menu
    int title_id;
    int main_menu_items[4]; // Resume, New Game, Settings, Quit
    int settings_items[4];  // Quality, Sound, Invert Y, Fullscreen
    int settings_values[4]; // Values for settings items
    
    // Window and initial size (for fullscreen toggle)
    SDL_Window *window;
    int window_width;
    int window_height;
    
    // References to other systems
    UISystem *ui;
    struct AudioSystem *audio;
    
    // Game settings
    GameSettings settings;
} MenuSystem;

// Function declarations
bool initMenuSystem(MenuSystem *menu, UISystem *ui, struct AudioSystem *audio, SDL_Window *window);
void updateMenuUI(MenuSystem *menu);
void handleMenuInput(MenuSystem *menu, SDL_Keycode key);
void cleanupMenuSystem(MenuSystem *menu);
void setMenuState(MenuSystem *menu, MenuState state);
bool isGamePaused(MenuSystem *menu);
bool isGameStarted(MenuSystem *menu);
void startNewGame(MenuSystem *menu);
void resumeGame(MenuSystem *menu);
void pauseGame(MenuSystem *menu);
GameSettings* getMenuSettings(MenuSystem *menu);

#endif // MENU_H