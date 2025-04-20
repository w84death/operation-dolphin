#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../include/game.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/terrain.h"
#include "../include/environment.h"
#include "../include/model.h"
#include "../include/config.h"
#include "../include/particles.h"
#include "../include/log.h"
#include "../include/audio.h"
#include "../include/vegetation.h"

// For the input system
static InputState input;

// Global pointer to current game state (for input handling)
GameState *game_state_ptr = NULL;

// Initialize the game state and resources
bool initGame(GameState* game) {
    // Set global pointer to current game state (for input handling)
    game_state_ptr = game;
    
    // Share the game state pointer with vegetation system
    setGameStatePointer(game);
    
    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Initialize game state
    game->running = true;
    game->fullscreen = false;
    game->game_started = false;
    game->game_paused = false;
    game->last_time = SDL_GetTicks() / 1000.0f;
    game->frame_count = 0;
    game->fps_last_time = SDL_GetTicks();
    
    // Initialize window size from config
    game->window_width = WINDOW_WIDTH;
    game->window_height = WINDOW_HEIGHT;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        logError("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // Create window with OpenGL context
    Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (game->fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }
    
    game->window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        game->window_width, game->window_height,
        window_flags
    );
    
    if (game->window == NULL) {
        logError("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Create OpenGL context
    game->gl_context = SDL_GL_CreateContext(game->window);
    if (game->gl_context == NULL) {
        logError("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        logWarning("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }
    
    // Initialize OpenGL
    glViewport(0, 0, game->window_width, game->window_height);
    
    glClearColor(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);
    glClearDepth(1.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Setup projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Calculate aspect ratio and set perspective
    float aspect = (float)game->window_width / (float)game->window_height;
    float fov = CAMERA_FOV;
    float near = CAMERA_NEAR;
    float far = CAMERA_FAR;
    float fH = tan(fov * 3.14159f / 360.0f) * near;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, near, far);
    
    // Initialize the player
    initPlayer(&game->player);
    
    // Initialize terrain system
    game->terrain = createFlatTerrain(TERRAIN_TILE_SIZE, TERRAIN_HEIGHT_SCALE);
    
    // Load vegetation textures
    if (!loadVegetationTextures()) {
        logError("Failed to load vegetation textures\n");
        return false;
    }
    
    // Initialize settings before creating vegetation (to ensure settings are applied)
    game->settings.sound_enabled = true;
    game->settings.high_terrain_features = true;
    game->settings.invert_y_axis = MOUSE_INVERT_Y_DEFAULT;
    game->settings.fullscreen = game->fullscreen;
    
    // Create vegetation with the appropriate density based on quality setting
    int vegetation_count = game->settings.high_terrain_features ? 
                          TERRAIN_MAX_FEATURES : TERRAIN_MAX_FEATURES / 2;
    createVegetation(vegetation_count, TERRAIN_TILE_SIZE);
    
    // Initialize day-night cycle
    initEnvironment();
    setBackgroundColor(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);
    
    // Initialize UI system with smaller font size for game UI
    if (!initUI(&game->game_ui, GAME_FONT_FILE, GAME_UI_FONT_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        logError("Failed to initialize game UI system\n");
        return false;
    } else {
        logInfo("Game UI system initialized successfully with font: %s (size %d)\n", 
               GAME_FONT_FILE, GAME_UI_FONT_SIZE);
    }
    
    // Initialize UI system with larger font size for menu UI
    if (!initUI(&game->menu_ui, GAME_FONT_FILE, MENU_FONT_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        logError("Failed to initialize menu UI system\n");
        cleanupUI(&game->game_ui);
        return false;
    } else {
        logInfo("Menu UI system initialized successfully with font: %s (size %d)\n", 
               GAME_FONT_FILE, MENU_FONT_SIZE);
    }
    
    // Initialize audio system
    if (!initAudio(&game->audio)) {
        logError("Failed to initialize audio system\n");
        return false;
    } else {
        logInfo("Audio system initialized successfully\n");
    }
    
    // Create UI elements
    SDL_Color primary_color = {UI_PRIMARY_COLOR_R, UI_PRIMARY_COLOR_G, UI_PRIMARY_COLOR_B, UI_PRIMARY_COLOR_A};
    SDL_Color secondary_color = {UI_SECONDARY_COLOR_R, UI_SECONDARY_COLOR_G, UI_SECONDARY_COLOR_B, UI_PRIMARY_COLOR_A};
    SDL_Color accent_color = {UI_ACCENT_COLOR_R, UI_ACCENT_COLOR_G, UI_ACCENT_COLOR_B, UI_PRIMARY_COLOR_A};
    
    // Create FPS text
    game->fps_text_id = createTextElement(&game->game_ui, "FPS: 0", WINDOW_WIDTH - 20, 20, primary_color, TEXT_ALIGN_RIGHT);
    
    // Create compass UI elements
    int compass_center_x = WINDOW_WIDTH / 2;
    // North indicator
    game->compass_n_id = createTextElement(&game->game_ui, COMPASS_NORTH, 
                                          compass_center_x, COMPASS_Y_POSITION, 
                                          primary_color, TEXT_ALIGN_CENTER);
    
    // East indicator
    game->compass_e_id = createTextElement(&game->game_ui, COMPASS_EAST, 
                                         compass_center_x + COMPASS_WIDTH/2 - COMPASS_CARDINAL_WIDTH/2, 
                                         COMPASS_Y_POSITION, 
                                         primary_color, TEXT_ALIGN_CENTER);
    
    // South indicator
    game->compass_s_id = createTextElement(&game->game_ui, COMPASS_SOUTH, 
                                         compass_center_x, COMPASS_Y_POSITION + COMPASS_HEIGHT, 
                                         primary_color, TEXT_ALIGN_CENTER);
    
    // West indicator
    game->compass_w_id = createTextElement(&game->game_ui, COMPASS_WEST, 
                                         compass_center_x - COMPASS_WIDTH/2 + COMPASS_CARDINAL_WIDTH/2, 
                                         COMPASS_Y_POSITION, 
                                         primary_color, TEXT_ALIGN_CENTER);
    
    // Indicator line (this will be a simple "-" positioned according to player direction)
    game->compass_indicator_id = createTextElement(&game->game_ui, "o", 
                                                 compass_center_x, COMPASS_Y_POSITION + COMPASS_LINE_HEIGHT, 
                                                 secondary_color, TEXT_ALIGN_CENTER);
    
    // Hide game UI initially (will be shown when game starts)
    setElementVisibility(&game->game_ui, game->fps_text_id, false);
    
    // Hide compass UI elements initially
    setElementVisibility(&game->game_ui, game->compass_n_id, false);
    setElementVisibility(&game->game_ui, game->compass_e_id, false);
    setElementVisibility(&game->game_ui, game->compass_s_id, false);
    setElementVisibility(&game->game_ui, game->compass_w_id, false);
    setElementVisibility(&game->game_ui, game->compass_indicator_id, false);
    
    // Initialize menu system
    initMenu(game);
    
    // Initialize input system
    initInput(&input);
    
    return true;
}

// Reset game to initial state for new game
void resetGame(GameState* game) {
    // Reset player position
    game->player.position_x = 0.0f;
    game->player.position_y = game->player.eye_height;
    game->player.position_z = 0.0f;
    game->player.velocity_x = 0.0f;
    game->player.velocity_y = 0.0f;
    game->player.velocity_z = 0.0f;
    game->player.yaw = 0.0f;
    game->player.pitch = 0.0f;
    
    // Reset game state flags
    game->game_started = true;
    game->game_paused = false;
    game->menu_state = MENU_NONE;
    
    // Play background music if sound is enabled
    if (game->settings.sound_enabled) {
        playBackgroundMusic(&game->audio);
    }
    
    // Update UI
    updateGameStats(game);
}

// Initialize the menu system
void initMenu(GameState* game) {
    // Initialize menu state
    game->menu_state = MENU_MAIN;
    game->selected_menu_item = 0;
    game->menu_item_count = 4; // Main menu has 4 items: Resume, New Game, Settings, Quit
    game->game_paused = false;
    
    // Initialize default settings
    game->settings.sound_enabled = true;
    game->settings.high_terrain_features = true;
    game->settings.invert_y_axis = MOUSE_INVERT_Y_DEFAULT;  // Initialize with default from config
    game->settings.fullscreen = game->fullscreen; // Initialize with current fullscreen state
    
    // Start playing menu music if sound is enabled
    if (game->settings.sound_enabled) {
        playMenuMusic(&game->audio);
    }
    
    // Define menu colors
    SDL_Color primary_color = {255, 255, 255, 255};
    
    // Create main menu title
    game->menu_title_id = createTextElement(&game->menu_ui, GAME_MENU_TITLE, 
                                           WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4, 
                                           primary_color, TEXT_ALIGN_CENTER);
    
    // Create main menu items - initially all primary_color, will update colors based on selection
    game->menu_items[0] = createTextElement(&game->menu_ui, GAME_MENU_RESUME, 
                                           WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 30, 
                                           primary_color, TEXT_ALIGN_CENTER);
    
    game->menu_items[1] = createTextElement(&game->menu_ui, GAME_MENU_NEWGAME, 
                                           WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 30, 
                                           primary_color, TEXT_ALIGN_CENTER);
    
    game->menu_items[2] = createTextElement(&game->menu_ui, GAME_MENU_SETTINGS, 
                                           WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 90, 
                                           primary_color, TEXT_ALIGN_CENTER);
    
    game->menu_items[3] = createTextElement(&game->menu_ui, GAME_MENU_QUIT, 
                                           WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 150, 
                                           primary_color, TEXT_ALIGN_CENTER);
    
    // Create settings menu items (initially hidden)
    game->settings_items[0] = createTextElement(&game->menu_ui, GAME_SETTINGS_QUALITY, 
                                              WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 60, 
                                              primary_color, TEXT_ALIGN_RIGHT);
    
    game->settings_items[1] = createTextElement(&game->menu_ui, GAME_SETTINGS_SOUND, 
                                              WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2, 
                                              primary_color, TEXT_ALIGN_RIGHT);
                                              
    // Add invert Y axis setting
    game->settings_items[2] = createTextElement(&game->menu_ui, GAME_SETTINGS_INVERT, 
                                              WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 60, 
                                              primary_color, TEXT_ALIGN_RIGHT);
                                              
    // Add fullscreen toggle setting
    game->settings_items[3] = createTextElement(&game->menu_ui, "FULLSCREEN", 
                                              WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 120, 
                                              primary_color, TEXT_ALIGN_RIGHT);
    
    // Create settings values (initially hidden)
    game->settings_values[0] = createTextElement(&game->menu_ui, GAME_SETTINGS_HIGH, 
                                               WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2 - 60, 
                                               primary_color, TEXT_ALIGN_LEFT);
    
    game->settings_values[1] = createTextElement(&game->menu_ui, "ON", 
                                               WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2, 
                                               primary_color, TEXT_ALIGN_LEFT);
                                               
    // Add invert Y axis value
    game->settings_values[2] = createTextElement(&game->menu_ui, "OFF", 
                                               WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2 + 60, 
                                               primary_color, TEXT_ALIGN_LEFT);
                                               
    // Add fullscreen toggle value
    game->settings_values[3] = createTextElement(&game->menu_ui, "OFF", 
                                               WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2 + 120, 
                                               primary_color, TEXT_ALIGN_LEFT);
    
    // Hide settings menu initially
    for (int i = 0; i < 4; i++) {
        setElementVisibility(&game->menu_ui, game->settings_items[i], false);
        setElementVisibility(&game->menu_ui, game->settings_values[i], false);
    }
    
    // Update menu colors based on selection
    updateMenuUI(game);
}

// Update menu text and visibility based on current state
void updateMenuUI(GameState* game) {
    SDL_Color primary_color = {UI_PRIMARY_COLOR_R, UI_PRIMARY_COLOR_G, UI_PRIMARY_COLOR_B, UI_PRIMARY_COLOR_A};
    SDL_Color secondary_color = {UI_SECONDARY_COLOR_R, UI_SECONDARY_COLOR_G, UI_SECONDARY_COLOR_B, UI_SECONDARY_COLOR_A};
    SDL_Color accent_color = {UI_ACCENT_COLOR_R, UI_ACCENT_COLOR_G, UI_ACCENT_COLOR_B, UI_PRIMARY_COLOR_A};
    
    if (game->menu_state == MENU_MAIN) {
        // Show title and main menu items
        setElementVisibility(&game->menu_ui, game->menu_title_id, true);
        
        // Show or hide "RESUME GAME" based on whether there's a game to resume
        setElementVisibility(&game->menu_ui, game->menu_items[0], game->game_started);
        
        // Determine starting index for menu items (skip RESUME if not available)
        int start_idx = game->game_started ? 0 : 1;
        
        // Show main menu, hide settings menu
        for (int i = start_idx; i < game->menu_item_count; i++) {
            setElementVisibility(&game->menu_ui, game->menu_items[i], true);
            
            // Set color based on selection
            if (i == game->selected_menu_item) {
                setElementColor(&game->menu_ui, game->menu_items[i], secondary_color);
            } else {
                setElementColor(&game->menu_ui, game->menu_items[i], primary_color);
            }
        }
        
        // Hide all settings menu items (both labels and values)
        for (int i = 0; i < 4; i++) {
            setElementVisibility(&game->menu_ui, game->settings_items[i], false);
            setElementVisibility(&game->menu_ui, game->settings_values[i], false);
        }
    }
    else if (game->menu_state == MENU_SETTINGS) {
        // Show title
        setElementVisibility(&game->menu_ui, game->menu_title_id, true);
        
        // Hide main menu
        for (int i = 0; i < game->menu_item_count; i++) {
            setElementVisibility(&game->menu_ui, game->menu_items[i], false);
        }
        
        // Show all settings items - including fullscreen which is only visible in settings
        for (int i = 0; i < 4; i++) {
            setElementVisibility(&game->menu_ui, game->settings_items[i], true);
            setElementVisibility(&game->menu_ui, game->settings_values[i], true);
            
            // Set color based on selection
            if (i == game->selected_menu_item) {
                setElementColor(&game->menu_ui, game->settings_items[i], secondary_color);
                setElementColor(&game->menu_ui, game->settings_values[i], secondary_color);
            } else {
                setElementColor(&game->menu_ui, game->settings_items[i], primary_color);
                setElementColor(&game->menu_ui, game->settings_values[i], primary_color);
            }
        }
        
        // Update settings values text
        updateTextElement(&game->menu_ui, game->settings_values[0], 
                          game->settings.high_terrain_features ? GAME_SETTINGS_HIGH : GAME_SETTINGS_LOW);
        updateTextElement(&game->menu_ui, game->settings_values[1], 
                          game->settings.sound_enabled ? GAME_SETTINGS_ON : GAME_SETTINGS_OFF);
        updateTextElement(&game->menu_ui, game->settings_values[2], 
                          game->settings.invert_y_axis ? GAME_SETTINGS_ON : GAME_SETTINGS_OFF);
        updateTextElement(&game->menu_ui, game->settings_values[3], 
                          game->settings.fullscreen ? GAME_SETTINGS_ON : GAME_SETTINGS_OFF);
    }
    else if (game->menu_state == MENU_NONE) {
        // Game is running, hide all menu elements
        setElementVisibility(&game->menu_ui, game->menu_title_id, false);
        
        for (int i = 0; i < game->menu_item_count; i++) {
            setElementVisibility(&game->menu_ui, game->menu_items[i], false);
        }
        
        // Hide all settings items
        for (int i = 0; i < 4; i++) {
            setElementVisibility(&game->menu_ui, game->settings_items[i], false);
            setElementVisibility(&game->menu_ui, game->settings_values[i], false);
        }
    }
    
    // Show/hide game UI based on whether the game is running
    bool show_game_ui = (game->menu_state == MENU_NONE && game->game_started);
    setElementVisibility(&game->game_ui, game->fps_text_id, show_game_ui);
    
    // Show/hide compass UI elements
    setElementVisibility(&game->game_ui, game->compass_n_id, show_game_ui);
    setElementVisibility(&game->game_ui, game->compass_e_id, show_game_ui);
    setElementVisibility(&game->game_ui, game->compass_s_id, show_game_ui);
    setElementVisibility(&game->game_ui, game->compass_w_id, show_game_ui);
    setElementVisibility(&game->game_ui, game->compass_indicator_id, show_game_ui);
}

// Handle menu navigation and selection
void handleMenuInput(GameState* game, SDL_Keycode key) {
    // Handle menu navigation and selection
    if (game->menu_state == MENU_MAIN) {
        // Get starting index (skip RESUME if it's not visible)
        int start_idx = game->game_started ? 0 : 1;
        
        switch (key) {
            case SDLK_UP:
                // Move selection up in the main menu
                game->selected_menu_item--;
                // Skip invisible resume option when game hasn't started
                if (!game->game_started && game->selected_menu_item < start_idx) {
                    game->selected_menu_item = game->menu_item_count - 1;
                } else if (game->selected_menu_item < 0) {
                    game->selected_menu_item = game->menu_item_count - 1;
                }
                updateMenuUI(game);
                break;
                
            case SDLK_DOWN:
                // Move selection down in the main menu
                game->selected_menu_item++;
                if (game->selected_menu_item >= game->menu_item_count) {
                    // When cycling from bottom to top, skip to the first visible option
                    game->selected_menu_item = start_idx;
                }
                updateMenuUI(game);
                break;
                
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Process main menu selection
                switch (game->selected_menu_item) {
                    case 0: // RESUME GAME
                        // Resume the current game if it was paused, or start a new one if not started yet
                        if (game->game_paused) {
                            game->game_paused = false;
                        } else if (!game->game_started) {
                            // If game wasn't started yet, start a new game
                            resetGame(game);
                        }
                        
                        // Switch from menu music to game music if sound is enabled
                        if (game->settings.sound_enabled && game->audio.in_menu_music) {
                            playBackgroundMusic(&game->audio);
                        }
                        
                        game->menu_state = MENU_NONE;
                        updateMenuUI(game);
                        break;

                    case 1: // NEW GAME
                        // Always reset and start a new game
                        resetGame(game);
                        
                        // Switch from menu music to game music if sound is enabled
                        if (game->settings.sound_enabled && game->audio.in_menu_music) {
                            playBackgroundMusic(&game->audio);
                        }
                        
                        game->menu_state = MENU_NONE;
                        updateMenuUI(game);
                        break;
                        
                    case 2: // SETTINGS
                        // Switch to settings menu
                        game->menu_state = MENU_SETTINGS;
                        game->selected_menu_item = 0;
                        updateMenuUI(game);
                        break;
                        
                    case 3: // QUIT
                        // Exit the game
                        game->running = false;
                        break;
                }
                break;
                
            case SDLK_ESCAPE:
                // If game was paused, resume it when pressing ESC again
                if (game->game_paused) {
                    game->game_paused = false;
                    game->menu_state = MENU_NONE;
                    updateMenuUI(game);
                } else {
                    // Otherwise exit the game from main menu
                    game->running = false;
                }
                break;
        }
    }
    else if (game->menu_state == MENU_SETTINGS) {
        switch (key) {
            case SDLK_UP:
                // Move selection up in the settings menu
                game->selected_menu_item--;
                if (game->selected_menu_item < 0) {
                    game->selected_menu_item = 3; // Settings has 4 items (0, 1, 2, 3)
                }
                updateMenuUI(game);
                break;
                
            case SDLK_DOWN:
                // Move selection down in the settings menu
                game->selected_menu_item++;
                if (game->selected_menu_item > 3) {
                    game->selected_menu_item = 0;
                }
                updateMenuUI(game);
                break;
                
            case SDLK_LEFT:
            case SDLK_RIGHT:
                // Toggle settings values
                switch (game->selected_menu_item) {
                    case 0: // TERRAIN FEATURES
                        game->settings.high_terrain_features = !game->settings.high_terrain_features;
                        
                        // Adjust vegetation based on setting
                        int new_count = game->settings.high_terrain_features ? 
                                       TERRAIN_MAX_FEATURES : TERRAIN_MAX_FEATURES / 2;
                        createVegetation(new_count, TERRAIN_TILE_SIZE);
                        break;
                        
                    case 1: // SOUND
                        game->settings.sound_enabled = !game->settings.sound_enabled;
                        
                        // Turn music on/off based on setting
                        if (game->settings.sound_enabled) {
                            if (game->game_started) {
                                playBackgroundMusic(&game->audio);
                            }
                        } else {
                            pauseBackgroundMusic(&game->audio);
                        }
                        break;
                        
                    case 2: // INVERT Y AXIS
                        game->settings.invert_y_axis = !game->settings.invert_y_axis;
                        break;
                        
                    case 3: // FULLSCREEN
                        // Toggle fullscreen using the same function as F11
                        game->fullscreen = !game->fullscreen;
                        game->settings.fullscreen = game->fullscreen;
                        
                        if (game->fullscreen) {
                            // Store the current window size before going fullscreen
                            SDL_GetWindowSize(game->window, &game->window_width, &game->window_height);
                            
                            // For fullscreen, use SDL_WINDOW_FULLSCREEN_DESKTOP for better compatibility
                            if (SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                                logError("Error switching to fullscreen: %s", SDL_GetError());
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
                            
                            logInfo("Switched to fullscreen: %dx%d", new_width, new_height);
                        } else {
                            // Return to windowed mode with original dimensions
                            if (SDL_SetWindowFullscreen(game->window, 0) != 0) {
                                logError("Error switching to windowed mode: %s", SDL_GetError());
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
                            
                            logInfo("Switched to windowed mode: %dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
                        }
                        break;
                }
                updateMenuUI(game);
                break;
                
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Same behavior as LEFT/RIGHT in settings - toggle the current item
                switch (game->selected_menu_item) {
                    case 0: // TERRAIN FEATURES
                        game->settings.high_terrain_features = !game->settings.high_terrain_features;
                        
                        // Adjust vegetation based on setting
                        int new_count = game->settings.high_terrain_features ? 
                                       TERRAIN_MAX_FEATURES : TERRAIN_MAX_FEATURES / 2;
                        createVegetation(new_count, TERRAIN_TILE_SIZE);
                        break;
                        
                    case 1: // SOUND
                        game->settings.sound_enabled = !game->settings.sound_enabled;
                        
                        // Turn music on/off based on setting
                        if (game->settings.sound_enabled) {
                            if (game->game_started) {
                                playBackgroundMusic(&game->audio);
                            }
                        } else {
                            pauseBackgroundMusic(&game->audio);
                        }
                        break;
                        
                    case 2: // INVERT Y AXIS
                        game->settings.invert_y_axis = !game->settings.invert_y_axis;
                        break;
                        
                    case 3: // FULLSCREEN
                        // Toggle fullscreen using the same function as F11
                        game->fullscreen = !game->fullscreen;
                        game->settings.fullscreen = game->fullscreen;
                        
                        if (game->fullscreen) {
                            // Store the current window size before going fullscreen
                            SDL_GetWindowSize(game->window, &game->window_width, &game->window_height);
                            
                            // For fullscreen, use SDL_WINDOW_FULLSCREEN_DESKTOP for better compatibility
                            if (SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                                logError("Error switching to fullscreen: %s", SDL_GetError());
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
                            
                            logInfo("Switched to fullscreen: %dx%d", new_width, new_height);
                        } else {
                            // Return to windowed mode with original dimensions
                            if (SDL_SetWindowFullscreen(game->window, 0) != 0) {
                                logError("Error switching to windowed mode: %s", SDL_GetError());
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
                            
                            logInfo("Switched to windowed mode: %dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
                        }
                        break;
                }
                updateMenuUI(game);
                break;
                
            case SDLK_ESCAPE:
                // Return to main menu
                game->menu_state = MENU_MAIN;
                game->selected_menu_item = 2; // Select SETTINGS item in main menu
                updateMenuUI(game);
                break;
        }
    }
    else if (game->menu_state == MENU_NONE && game->game_started) {
        // If in game, ESC key should pause and show the main menu
        if (key == SDLK_ESCAPE) {
            game->menu_state = MENU_MAIN;
            game->game_paused = true;
            game->selected_menu_item = 0; // Select RESUME GAME by default
            updateMenuUI(game);
            
            // Switch from gameplay music to menu music if sound is enabled
            if (game->settings.sound_enabled && !game->audio.in_menu_music) {
                playMenuMusic(&game->audio);
            }
        }
    }
}

// Update game statistics UI elements
void updateGameStats(GameState* game) {
    char buffer[50];
    
    // Update FPS text (updated once per second)
    Uint32 current_time = SDL_GetTicks();
    
    if (current_time - game->fps_last_time >= 1000) {
        // Calculate the actual FPS based on frames rendered since last update
        int fps = game->frame_count;
        snprintf(buffer, sizeof(buffer), "FPS: %d", fps);
        updateTextElement(&game->game_ui, game->fps_text_id, buffer);
        
        // Reset frame counter and update the last time
        game->frame_count = 0;
        game->fps_last_time = current_time;
    }
}

// Handle SDL events
void handleEvents(GameState* game) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->running = false;
        } 
        else if (event.type == SDL_KEYDOWN) {
            handleKeyDown(&input, event.key.keysym.sym, game);
        } 
        else if (event.type == SDL_KEYUP) {
            handleKeyUp(&input, event.key.keysym.sym);
        }
        else if (event.type == SDL_MOUSEMOTION) {
            // Handle mouse motion for camera control
            handleMouseMotion(&input, event.motion.x, event.motion.y, 
                             event.motion.xrel, event.motion.yrel);
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            // Handle mouse button down
            handleMouseButton(&input, event.button.button - 1, true);
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            // Handle mouse button up
            handleMouseButton(&input, event.button.button - 1, false);
        }
    }
}

// Update game state
void updateGame(GameState* game, float delta_time) {
    // Process input
    processInput(&input, game, &game->player, delta_time);
    
    // Only update game simulation if game is running and not in a menu
    if (game->menu_state == MENU_NONE && game->game_started) {
        // Update player
        updatePlayer(&game->player, delta_time);
        
        // Handle foliage cutting if player is in cutting animation
        if (game->player.is_cutting) {
            cutMediumFoliage(&game->player);
        }
        
        // Update particles
        updateParticles(delta_time);
        
        // Update day-night cycle
        updateDayNightCycle(delta_time);
        
        // Update compass UI
        updateCompassUI(game);
    }
    
    // Always update music system
    updateMusicSystem(&game->audio, delta_time);
    
    // Update game statistics UI
    updateGameStats(game);
}

// Render the game
void renderGame(GameState* game) {
    // Clear the screen and setup environment for current time of day
    TimeOfDay current_tod = getCurrentTimeOfDay();
    setupFogForTimeOfDay(current_tod, FOG_START, FOG_END);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // First-person camera setup using player's position and rotation
    if (game->menu_state == MENU_NONE && game->game_started) {
        // In-game camera - use player's position and orientation
        // Apply player pitch rotation
        glRotatef(game->player.pitch, 1.0f, 0.0f, 0.0f);
        
        // Apply player yaw rotation
        glRotatef(game->player.yaw, 0.0f, 1.0f, 0.0f);
        
        // Translate to negative of player position
        glTranslatef(-game->player.position_x, 
                    -game->player.position_y, 
                    -game->player.position_z);
    } else {
        // Menu camera - show elevated view of the jungle
        // Use fmodf for floating point modulo operation
        float cam_yaw = fmodf((SDL_GetTicks() * MENU_CAMERA_ROTATION_SPEED), 360.0f); // Slowly rotate camera
        
        // Apply pitch to look down at terrain
        glRotatef(MENU_CAMERA_PITCH, 1.0f, 0.0f, 0.0f);
        
        // Apply rotating yaw for a nice overview effect
        glRotatef(cam_yaw, 0.0f, 1.0f, 0.0f);
        
        // Position high above the terrain
        glTranslatef(0.0f, -MENU_CAMERA_HEIGHT, 0.0f);
    }
    
    // Setup lighting based on time of day
    setupLightingForTimeOfDay(current_tod);
    
    // Render terrain
    renderTerrain(game->terrain);
    
    // Render vegetation
    renderVegetation();
    
    // Render particles
    renderParticles();
    
    // Restore model-view matrix for weapon rendering
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Render player's weapon (if game is started)
    if (game->game_started && game->menu_state == MENU_NONE) {
        renderWeapon(&game->player);
    }
    
    // Render UI systems - game UI and menu UI
    renderUI(&game->game_ui);
    renderUI(&game->menu_ui);
    
    // Swap buffers
    SDL_GL_SwapWindow(game->window);
    
    // Increment frame counter
    game->frame_count++;
}

// Main game loop
void gameLoop(GameState* game) {
    Uint32 frame_start;
    float current_time, delta_time;
    
    // Initial UI update
    updateGameStats(game);
    
    while (game->running) {
        frame_start = SDL_GetTicks();
        
        // Calculate delta time
        current_time = SDL_GetTicks() / 1000.0f;
        delta_time = current_time - game->last_time;
        game->last_time = current_time;
        
        // Handle input events
        handleEvents(game);
        
        // Update game state
        updateGame(game, delta_time);
        
        // Render the game
        renderGame(game);
        
        // Cap framerate to ~60 FPS
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 16) {
            SDL_Delay(16 - frame_time);
        }
    }
}

// Clean up game resources
void cleanupGame(GameState* game) {
    // Clean up player resources
    cleanupPlayer(&game->player);
    
    // Clean up terrain resources
    cleanupTerrain(game->terrain);
    
    // Free vegetation
    cleanupVegetation();
    
    // Clean up audio system
    cleanupAudio(&game->audio);
    
    // Clean up UI system
    cleanupUI(&game->game_ui);
    cleanupUI(&game->menu_ui);
    
    // Clean up SDL resources
    SDL_GL_DeleteContext(game->gl_context);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
}

// Update compass UI based on player's direction
void updateCompassUI(GameState* game) {
    if (game->menu_state == MENU_NONE && game->game_started) {
        int compass_center_x = WINDOW_WIDTH / 2;
        
        // Calculate the position of the compass indicator based on player's yaw
        float relative_angle = game->player.yaw;
        
        // Normalize between 0-360 degrees
        while (relative_angle < 0) relative_angle += 360.0f;
        while (relative_angle >= 360) relative_angle -= 360.0f;
        
        // Convert to a position in the compass UI (-1 to +1 range, where 0 is north)
        float position = relative_angle / 180.0f - 1.0f; // Maps 0-360 to -1 to 1
        
        // Calculate the X position on screen for the indicator
        int indicator_x = compass_center_x + (int)(position * COMPASS_WIDTH/2);
        
        // Ensure the indicator stays within compass bounds
        if (indicator_x < compass_center_x - COMPASS_WIDTH/2) 
            indicator_x = compass_center_x - COMPASS_WIDTH/2;
        if (indicator_x > compass_center_x + COMPASS_WIDTH/2) 
            indicator_x = compass_center_x + COMPASS_WIDTH/2;
            
        // Update the compass indicator position
        setElementPosition(&game->game_ui, game->compass_indicator_id, indicator_x, COMPASS_Y_POSITION + COMPASS_LINE_HEIGHT);
    }
}