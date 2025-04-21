#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "../include/menu.h"
#include "../include/config.h"
#include "../include/ui.h"
#include "../include/log.h"
#include "../include/terrain.h"

// Define the AudioSystem structure here to match the one in game.h
typedef struct AudioSystem {
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

// Forward declarations for audio functions
void playMenuMusic(AudioSystem* audio);
void playBackgroundMusic(AudioSystem* audio);
void pauseBackgroundMusic(AudioSystem* audio);

// Forward declaration for function to create vegetation
extern void createVegetation(int count, float terrain_size);

// Initialize the menu system
bool initMenuSystem(MenuSystem *menu, UISystem *ui, AudioSystem *audio, SDL_Window *window) {
    // Initialize menu state
    menu->state = MENU_MAIN;
    menu->selected_item = 0;
    menu->item_count = 4; // Main menu has 4 items: Resume, New Game, Settings, Quit
    menu->game_paused = false;
    menu->game_started = false;
    
    // Store references
    menu->ui = ui;
    menu->audio = audio;
    menu->window = window;
    
    // Initialize default settings
    menu->settings.sound_enabled = true;
    menu->settings.high_terrain_features = true;
    menu->settings.invert_y_axis = MOUSE_INVERT_Y_DEFAULT;
    menu->settings.fullscreen = false; // Default to windowed mode
    
    // Get initial window size
    SDL_GetWindowSize(window, &menu->window_width, &menu->window_height);
    
    // Start playing menu music if sound is enabled
    if (menu->settings.sound_enabled) {
        playMenuMusic(audio);
    }
    
    // Define menu colors
    SDL_Color primary_color = {UI_PRIMARY_COLOR_R, UI_PRIMARY_COLOR_G, UI_PRIMARY_COLOR_B, UI_PRIMARY_COLOR_A};
    
    // Create main menu title
    menu->title_id = createImageElement(ui, "textures/ui/logo.tga", 
                                      MENU_TITLE_X_POS, MENU_TITLE_Y_POS, 
                                      0, 0, TEXT_ALIGN_CENTER);
    
    // Create main menu items - initially all primary_color, will update colors based on selection
    menu->main_menu_items[0] = createTextElement(ui, GAME_MENU_RESUME, 
                                               MENU_ITEM_X_POS, MENU_FIRST_ITEM_Y_POS, 
                                               primary_color, TEXT_ALIGN_CENTER);
    
    menu->main_menu_items[1] = createTextElement(ui, GAME_MENU_NEWGAME, 
                                               MENU_ITEM_X_POS, MENU_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING, 
                                               primary_color, TEXT_ALIGN_CENTER);
    
    menu->main_menu_items[2] = createTextElement(ui, GAME_MENU_SETTINGS, 
                                               MENU_ITEM_X_POS, MENU_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING * 2, 
                                               primary_color, TEXT_ALIGN_CENTER);
    
    menu->main_menu_items[3] = createTextElement(ui, GAME_MENU_QUIT, 
                                               MENU_ITEM_X_POS, MENU_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING * 3, 
                                               primary_color, TEXT_ALIGN_CENTER);
    
    // Create settings menu items (initially hidden)
    menu->settings_items[0] = createTextElement(ui, GAME_SETTINGS_QUALITY, 
                                              MENU_SETTINGS_LABEL_X_POS, MENU_SETTINGS_FIRST_ITEM_Y_POS, 
                                              primary_color, TEXT_ALIGN_RIGHT);
    
    menu->settings_items[1] = createTextElement(ui, GAME_SETTINGS_SOUND, 
                                              MENU_SETTINGS_LABEL_X_POS, 
                                              MENU_SETTINGS_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING, 
                                              primary_color, TEXT_ALIGN_RIGHT);
                                              
    // Add invert Y axis setting
    menu->settings_items[2] = createTextElement(ui, GAME_SETTINGS_INVERT, 
                                              MENU_SETTINGS_LABEL_X_POS, 
                                              MENU_SETTINGS_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING * 2, 
                                              primary_color, TEXT_ALIGN_RIGHT);
                                              
    // Add fullscreen toggle setting
    menu->settings_items[3] = createTextElement(ui, "FULLSCREEN", 
                                              MENU_SETTINGS_LABEL_X_POS, 
                                              MENU_SETTINGS_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING * 3, 
                                              primary_color, TEXT_ALIGN_RIGHT);
    
    // Create settings values (initially hidden)
    menu->settings_values[0] = createTextElement(ui, GAME_SETTINGS_HIGH, 
                                               MENU_SETTINGS_VALUE_X_POS, 
                                               MENU_SETTINGS_FIRST_ITEM_Y_POS, 
                                               primary_color, TEXT_ALIGN_LEFT);
    
    menu->settings_values[1] = createTextElement(ui, GAME_SETTINGS_ON, 
                                               MENU_SETTINGS_VALUE_X_POS, 
                                               MENU_SETTINGS_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING, 
                                               primary_color, TEXT_ALIGN_LEFT);
                                               
    // Add invert Y axis value
    menu->settings_values[2] = createTextElement(ui, GAME_SETTINGS_OFF, 
                                               MENU_SETTINGS_VALUE_X_POS, 
                                               MENU_SETTINGS_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING * 2, 
                                               primary_color, TEXT_ALIGN_LEFT);
                                               
    // Add fullscreen toggle value
    menu->settings_values[3] = createTextElement(ui, GAME_SETTINGS_OFF, 
                                               MENU_SETTINGS_VALUE_X_POS, 
                                               MENU_SETTINGS_FIRST_ITEM_Y_POS + MENU_ITEM_Y_SPACING * 3, 
                                               primary_color, TEXT_ALIGN_LEFT);
    
    // Hide settings menu initially
    for (int i = 0; i < 4; i++) {
        setElementVisibility(ui, menu->settings_items[i], false);
        setElementVisibility(ui, menu->settings_values[i], false);
    }
    
    // Update menu colors based on selection
    updateMenuSystemUI(menu);
    
    return true;
}

// Update menu text and visibility based on current state
void updateMenuSystemUI(MenuSystem *menu) {
    SDL_Color primary_color = {UI_PRIMARY_COLOR_R, UI_PRIMARY_COLOR_G, UI_PRIMARY_COLOR_B, UI_PRIMARY_COLOR_A};
    SDL_Color secondary_color = {UI_SECONDARY_COLOR_R, UI_SECONDARY_COLOR_G, UI_SECONDARY_COLOR_B, UI_SECONDARY_COLOR_A};
    SDL_Color accent_color = {UI_ACCENT_COLOR_R, UI_ACCENT_COLOR_G, UI_ACCENT_COLOR_B, UI_ACCENT_COLOR_A};
    
    UISystem *ui = menu->ui;
    
    if (menu->state == MENU_MAIN) {
        // Show title and main menu items
        setElementVisibility(ui, menu->title_id, true);
        
        // Show or hide "RESUME GAME" based on whether there's a game to resume
        setElementVisibility(ui, menu->main_menu_items[0], menu->game_started);
        
        // Determine starting index for menu items (skip RESUME if not available)
        int start_idx = menu->game_started ? 0 : 1;
        
        // Show main menu, hide settings menu
        for (int i = start_idx; i < menu->item_count; i++) {
            setElementVisibility(ui, menu->main_menu_items[i], true);
            
            // Set color based on selection
            if (i == menu->selected_item) {
                setElementColor(ui, menu->main_menu_items[i], secondary_color);
            } else {
                setElementColor(ui, menu->main_menu_items[i], primary_color);
            }
        }
        
        // Hide all settings menu items (both labels and values)
        for (int i = 0; i < 4; i++) {
            setElementVisibility(ui, menu->settings_items[i], false);
            setElementVisibility(ui, menu->settings_values[i], false);
        }
    }
    else if (menu->state == MENU_SETTINGS) {
        // Show title
        setElementVisibility(ui, menu->title_id, true);
        
        // Hide main menu
        for (int i = 0; i < menu->item_count; i++) {
            setElementVisibility(ui, menu->main_menu_items[i], false);
        }
        
        // Show all settings items - including fullscreen which is only visible in settings
        for (int i = 0; i < 4; i++) {
            setElementVisibility(ui, menu->settings_items[i], true);
            setElementVisibility(ui, menu->settings_values[i], true);
            
            // Set color based on selection
            if (i == menu->selected_item) {
                setElementColor(ui, menu->settings_items[i], secondary_color);
                setElementColor(ui, menu->settings_values[i], secondary_color);
            } else {
                setElementColor(ui, menu->settings_items[i], primary_color);
                setElementColor(ui, menu->settings_values[i], primary_color);
            }
        }
        
        // Update settings values text
        updateTextElement(ui, menu->settings_values[0], 
                         menu->settings.high_terrain_features ? GAME_SETTINGS_HIGH : GAME_SETTINGS_LOW);
        updateTextElement(ui, menu->settings_values[1], 
                         menu->settings.sound_enabled ? GAME_SETTINGS_ON : GAME_SETTINGS_OFF);
        updateTextElement(ui, menu->settings_values[2], 
                         menu->settings.invert_y_axis ? GAME_SETTINGS_ON : GAME_SETTINGS_OFF);
        updateTextElement(ui, menu->settings_values[3], 
                         menu->settings.fullscreen ? GAME_SETTINGS_ON : GAME_SETTINGS_OFF);
    }
    else if (menu->state == MENU_NONE) {
        // Game is running, hide all menu elements
        setElementVisibility(ui, menu->title_id, false);
        
        for (int i = 0; i < menu->item_count; i++) {
            setElementVisibility(ui, menu->main_menu_items[i], false);
        }
        
        // Hide all settings items
        for (int i = 0; i < 4; i++) {
            setElementVisibility(ui, menu->settings_items[i], false);
            setElementVisibility(ui, menu->settings_values[i], false);
        }
    }
}

// Handle menu navigation and selection
void handleMenuSystemInput(MenuSystem *menu, SDL_Keycode key) {
    UISystem *ui = menu->ui;
    
    // Handle menu navigation and selection
    if (menu->state == MENU_MAIN) {
        // Get starting index (skip RESUME if it's not visible)
        int start_idx = menu->game_started ? 0 : 1;
        
        switch (key) {
            case SDLK_UP:
                // Move selection up in the main menu
                menu->selected_item--;
                // Skip invisible resume option when game hasn't started
                if (!menu->game_started && menu->selected_item < start_idx) {
                    menu->selected_item = menu->item_count - 1;
                } else if (menu->selected_item < 0) {
                    menu->selected_item = menu->item_count - 1;
                }
                updateMenuSystemUI(menu);
                break;
                
            case SDLK_DOWN:
                // Move selection down in the main menu
                menu->selected_item++;
                if (menu->selected_item >= menu->item_count) {
                    // When cycling from bottom to top, skip to the first visible option
                    menu->selected_item = start_idx;
                }
                updateMenuSystemUI(menu);
                break;
                
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Process main menu selection
                switch (menu->selected_item) {
                    case 0: // RESUME GAME
                        resumeGame(menu);
                        break;

                    case 1: // NEW GAME
                        startNewGame(menu);
                        break;
                        
                    case 2: // SETTINGS
                        // Switch to settings menu
                        menu->state = MENU_SETTINGS;
                        menu->selected_item = 0;
                        updateMenuSystemUI(menu);
                        break;
                        
                    case 3: // QUIT
                        // Set running to false (will be handled by game loop)
                        // We don't directly set running to false here because the menu system
                        // doesn't have direct access to the game's running variable
                        menu->state = MENU_NONE;  // Signal to exit
                        break;
                }
                break;
                
            case SDLK_ESCAPE:
                // If game was paused, resume it when pressing ESC again
                if (menu->game_paused) {
                    resumeGame(menu);
                } else {
                    // Otherwise set signal to exit the game from main menu
                    menu->state = MENU_NONE;  // Signal to exit
                }
                break;
        }
    }
    else if (menu->state == MENU_SETTINGS) {
        switch (key) {
            case SDLK_UP:
                // Move selection up in the settings menu
                menu->selected_item--;
                if (menu->selected_item < 0) {
                    menu->selected_item = 3; // Settings has 4 items (0, 1, 2, 3)
                }
                updateMenuSystemUI(menu);
                break;
                
            case SDLK_DOWN:
                // Move selection down in the settings menu
                menu->selected_item++;
                if (menu->selected_item > 3) {
                    menu->selected_item = 0;
                }
                updateMenuSystemUI(menu);
                break;
                
            case SDLK_LEFT:
            case SDLK_RIGHT:
                // Toggle settings values
                switch (menu->selected_item) {
                    case 0: // TERRAIN FEATURES
                        menu->settings.high_terrain_features = !menu->settings.high_terrain_features;
                        
                        // Adjust vegetation based on setting
                        int new_count = menu->settings.high_terrain_features ? 
                                      TERRAIN_MAX_FEATURES : TERRAIN_MAX_FEATURES / 2;
                        createVegetation(new_count, TERRAIN_TILE_SIZE);
                        break;
                        
                    case 1: // SOUND
                        menu->settings.sound_enabled = !menu->settings.sound_enabled;
                        
                        // Turn music on/off based on setting
                        if (menu->settings.sound_enabled) {
                            if (menu->game_started) {
                                playBackgroundMusic(menu->audio);
                            }
                        } else {
                            pauseBackgroundMusic(menu->audio);
                        }
                        break;
                        
                    case 2: // INVERT Y AXIS
                        menu->settings.invert_y_axis = !menu->settings.invert_y_axis;
                        break;
                        
                    case 3: // FULLSCREEN
                        // Toggle fullscreen using the same code as LEFT/RIGHT
                        menu->settings.fullscreen = !menu->settings.fullscreen;
                        
                        if (menu->settings.fullscreen) {
                            // Store the current window size before going fullscreen
                            SDL_GetWindowSize(menu->window, &menu->window_width, &menu->window_height);
                            
                            // For fullscreen, use SDL_WINDOW_FULLSCREEN_DESKTOP for better compatibility
                            if (SDL_SetWindowFullscreen(menu->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                                logError("Error switching to fullscreen: %s", SDL_GetError());
                            }
                            
                            // Get the new resolution after switching to fullscreen
                            int new_width, new_height;
                            SDL_GetWindowSize(menu->window, &new_width, &new_height);
                            
                            // Update UI positions based on new resolution
                            repositionUI(menu->ui, new_width, new_height);
                            
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
                            
                            logInfo("Switched to fullscreen: %dx%d", new_width, new_height);
                        } else {
                            // Return to windowed mode with original dimensions
                            if (SDL_SetWindowFullscreen(menu->window, 0) != 0) {
                                logError("Error switching to windowed mode: %s", SDL_GetError());
                            }
                            
                            // Restore original window size
                            SDL_SetWindowSize(menu->window, WINDOW_WIDTH, WINDOW_HEIGHT);
                            
                            // Update UI positions based on original resolution
                            repositionUI(menu->ui, WINDOW_WIDTH, WINDOW_HEIGHT);
                            
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
                            
                            logInfo("Switched to windowed mode: %dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
                        }
                        break;
                }
                updateMenuSystemUI(menu);
                break;
                
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Same behavior as LEFT/RIGHT in settings - toggle the current item
                switch (menu->selected_item) {
                    case 0: // TERRAIN FEATURES
                        menu->settings.high_terrain_features = !menu->settings.high_terrain_features;
                        
                        // Adjust vegetation based on setting
                        int new_count = menu->settings.high_terrain_features ? 
                                       TERRAIN_MAX_FEATURES : TERRAIN_MAX_FEATURES / 2;
                        createVegetation(new_count, TERRAIN_TILE_SIZE);
                        break;
                        
                    case 1: // SOUND
                        menu->settings.sound_enabled = !menu->settings.sound_enabled;
                        
                        // Turn music on/off based on setting
                        if (menu->settings.sound_enabled) {
                            if (menu->game_started) {
                                playBackgroundMusic(menu->audio);
                            }
                        } else {
                            pauseBackgroundMusic(menu->audio);
                        }
                        break;
                        
                    case 2: // INVERT Y AXIS
                        menu->settings.invert_y_axis = !menu->settings.invert_y_axis;
                        break;
                        
                    case 3: // FULLSCREEN
                        // Toggle fullscreen
                        menu->settings.fullscreen = !menu->settings.fullscreen;
                        
                        if (menu->settings.fullscreen) {
                            // Same code as in LEFT/RIGHT for fullscreen
                            SDL_GetWindowSize(menu->window, &menu->window_width, &menu->window_height);
                            if (SDL_SetWindowFullscreen(menu->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                                logError("Error switching to fullscreen: %s", SDL_GetError());
                            }
                            
                            int new_width, new_height;
                            SDL_GetWindowSize(menu->window, &new_width, &new_height);
                            repositionUI(menu->ui, new_width, new_height);
                            glViewport(0, 0, new_width, new_height);
                            
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
                            
                            logInfo("Switched to fullscreen: %dx%d", new_width, new_height);
                        } else {
                            // Same code as in LEFT/RIGHT for windowed mode
                            if (SDL_SetWindowFullscreen(menu->window, 0) != 0) {
                                logError("Error switching to windowed mode: %s", SDL_GetError());
                            }
                            
                            SDL_SetWindowSize(menu->window, WINDOW_WIDTH, WINDOW_HEIGHT);
                            repositionUI(menu->ui, WINDOW_WIDTH, WINDOW_HEIGHT);
                            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
                            
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
                            
                            logInfo("Switched to windowed mode: %dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
                        }
                        break;
                }
                updateMenuSystemUI(menu);
                break;
                
            case SDLK_ESCAPE:
                // Return to main menu
                menu->state = MENU_MAIN;
                menu->selected_item = 2; // Select SETTINGS item in main menu
                updateMenuSystemUI(menu);
                break;
        }
    }
}

// Clean up menu resources
void cleanupMenuSystem(MenuSystem *menu) {
    // Nothing to clean up specifically for the menu system
    // UI elements are handled by the UI system cleanup
}

// Set the menu state
void setMenuState(MenuSystem *menu, MenuState state) {
    menu->state = state;
    updateMenuSystemUI(menu);
}

// Check if the game is currently paused
bool isGamePaused(MenuSystem *menu) {
    return menu->game_paused;
}

// Check if the game has been started
bool isGameStarted(MenuSystem *menu) {
    return menu->game_started;
}

// Start a new game
void startNewGame(MenuSystem *menu) {
    // Set game state flags
    menu->game_started = true;
    menu->game_paused = false;
    menu->state = MENU_NONE;
    
    // Switch from menu music to game music if sound is enabled
    if (menu->settings.sound_enabled && menu->audio->in_menu_music) {
        playBackgroundMusic(menu->audio);
    }
    
    // Update UI
    updateMenuSystemUI(menu);
}

// Resume game
void resumeGame(MenuSystem *menu) {
    if (menu->game_paused) {
        menu->game_paused = false;
    }
    
    // Switch from menu music to game music if sound is enabled
    if (menu->settings.sound_enabled && menu->audio->in_menu_music) {
        playBackgroundMusic(menu->audio);
    }
    
    menu->state = MENU_NONE;
    updateMenuSystemUI(menu);
}

// Pause game and show menu
void pauseGame(MenuSystem *menu) {
    menu->state = MENU_MAIN;
    menu->game_paused = true;
    menu->selected_item = 0; // Select RESUME GAME by default
    
    // Switch from gameplay music to menu music if sound is enabled
    if (menu->settings.sound_enabled && !menu->audio->in_menu_music) {
        playMenuMusic(menu->audio);
    }
    
    updateMenuSystemUI(menu);
}

// Get a pointer to the menu settings
GameSettings* getMenuSettings(MenuSystem *menu) {
    return &menu->settings;
}