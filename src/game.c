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

// For the input system
static InputState input;

// Type definition for vegetation
typedef struct {
    float x, y, z;     // Position
    float width, height; // Dimensions
    int texture_index;   // Which texture to use
    bool active;         // Whether this vegetation is visible
} Vegetation;

// Vegetation array
static Vegetation* vegetation = NULL;
static int vegetation_count = 0;
static GLuint vegetation_textures[5]; // For different types of vegetation

// Initialize audio system
bool initAudio(AudioSystem* audio) {
    // Initialize SDL_mixer
    if (Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNKSIZE) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        audio->initialized = false;
        return false;
    }
    
    // Set up music tracks array
    audio->num_tracks = NUM_MUSIC_TRACKS;
    audio->music_tracks = (Mix_Music**)malloc(audio->num_tracks * sizeof(Mix_Music*));
    audio->music_filenames = (char**)malloc(audio->num_tracks * sizeof(char*));
    
    // Define music filenames
    audio->music_filenames[0] = MUSIC_TRACK_1;
    audio->music_filenames[1] = MUSIC_TRACK_2;
    audio->music_filenames[2] = MUSIC_TRACK_3;
    
    // Load menu music separately
    audio->menu_music = Mix_LoadMUS(MENU_MUSIC_FILE);
    if (!audio->menu_music) {
        printf("Failed to load menu music! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        printf("Menu music loaded successfully: %s\n", MENU_MUSIC_FILE);
    }
    
    // Load all music tracks
    bool loaded_at_least_one = false;
    for (int i = 0; i < audio->num_tracks; i++) {
        audio->music_tracks[i] = Mix_LoadMUS(audio->music_filenames[i]);
        if (!audio->music_tracks[i]) {
            printf("Failed to load music track %d (%s)! SDL_mixer Error: %s\n", 
                   i, audio->music_filenames[i], Mix_GetError());
        } else {
            printf("Music track %d loaded successfully: %s\n", i, audio->music_filenames[i]);
            loaded_at_least_one = true;
        }
    }
    
    // Set up gameplay playlist (all tracks for now)
    audio->num_gameplay_tracks = audio->num_tracks;
    audio->gameplay_playlist = (int*)malloc(audio->num_gameplay_tracks * sizeof(int));
    for (int i = 0; i < audio->num_gameplay_tracks; i++) {
        audio->gameplay_playlist[i] = i;  // Add all tracks to gameplay playlist
    }
    
    // Initialize track indices
    audio->current_track = 0;
    audio->current_gameplay_track = 0;
    audio->track_switch_timer = 0.0f;
    audio->in_menu_music = false;
    
    // Set default volume
    Mix_VolumeMusic(MUSIC_VOLUME);
    
    audio->music_playing = false;
    audio->initialized = loaded_at_least_one || audio->menu_music;
    
    // Return success if at least one track or menu music loaded
    return audio->initialized;
}

// Play a specific music track
bool playMusicTrack(AudioSystem* audio, int track_index) {
    // Only attempt to play if audio is initialized and track exists
    if (audio->initialized && track_index >= 0 && track_index < audio->num_tracks && 
        audio->music_tracks[track_index]) {
        
        // Stop any currently playing music
        if (Mix_PlayingMusic()) {
            Mix_HaltMusic();
        }
        
        // Play the track
        if (Mix_PlayMusic(audio->music_tracks[track_index], -1) == -1) {
            printf("Failed to play music track %d! SDL_mixer Error: %s\n", 
                   track_index, Mix_GetError());
            audio->music_playing = false;
            return false;
        } else {
            audio->current_track = track_index;
            audio->music_playing = true;
            printf("Playing music track %d: %s\n", 
                   track_index, audio->music_filenames[track_index]);
            return true;
        }
    }
    return false;
}

// Play menu music
bool playMenuMusic(AudioSystem* audio) {
    // Only attempt to play if audio is initialized and menu music is loaded
    if (audio->initialized && audio->menu_music) {
        // Stop any currently playing music
        if (Mix_PlayingMusic()) {
            Mix_HaltMusic();
        }
        
        // Play menu music
        if (Mix_PlayMusic(audio->menu_music, -1) == -1) {
            printf("Failed to play menu music! SDL_mixer Error: %s\n", Mix_GetError());
            audio->music_playing = false;
            audio->in_menu_music = false;
            return false;
        } else {
            audio->music_playing = true;
            audio->in_menu_music = true;
            printf("Playing menu music\n");
            return true;
        }
    }
    return false;
}

// Play next track in the gameplay playlist
bool playNextGameplayTrack(AudioSystem* audio) {
    // Move to next track in playlist
    audio->current_gameplay_track = (audio->current_gameplay_track + 1) % audio->num_gameplay_tracks;
    int track_index = audio->gameplay_playlist[audio->current_gameplay_track];
    
    // Reset the track switch timer
    audio->track_switch_timer = 0.0f;
    
    // Play the track
    return playMusicTrack(audio, track_index);
}

// Play a random track from the gameplay playlist
bool playRandomGameplayTrack(AudioSystem* audio) {
    if (audio->num_gameplay_tracks > 0) {
        // Pick a random track from playlist that's different from current
        int random_index;
        do {
            random_index = rand() % audio->num_gameplay_tracks;
        } while (audio->num_gameplay_tracks > 1 && 
                 audio->gameplay_playlist[random_index] == audio->current_track);
        
        audio->current_gameplay_track = random_index;
        int track_index = audio->gameplay_playlist[random_index];
        
        // Reset the track switch timer
        audio->track_switch_timer = 0.0f;
        
        // Play the track
        return playMusicTrack(audio, track_index);
    }
    return false;
}

// Play background music
void playBackgroundMusic(AudioSystem* audio) {
    // Start with the first gameplay track
    if (audio->initialized && !audio->music_playing) {
        playRandomGameplayTrack(audio);
    }
}

// Pause background music
void pauseBackgroundMusic(AudioSystem* audio) {
    if (audio->initialized && audio->music_playing) {
        Mix_PauseMusic();
        audio->music_playing = false;
    }
}

// Resume background music
void resumeBackgroundMusic(AudioSystem* audio) {
    if (audio->initialized && !audio->music_playing && Mix_PausedMusic()) {
        Mix_ResumeMusic();
        audio->music_playing = true;
    }
}

// Update music system - check for track completion and handle switching
void updateMusicSystem(AudioSystem* audio, float delta_time) {
    if (audio->initialized && audio->music_playing) {
        // Update track timer
        audio->track_switch_timer += delta_time;
        
        // Check if it's time to switch tracks or if the music stopped playing
        if (audio->track_switch_timer >= TRACK_SWITCH_TIME || !Mix_PlayingMusic()) {
            playRandomGameplayTrack(audio);
        }
    }
}

// Cleanup audio system
void cleanupAudio(AudioSystem* audio) {
    if (audio->initialized) {
        // Free music
        for (int i = 0; i < audio->num_tracks; i++) {
            if (audio->music_tracks[i]) {
                Mix_FreeMusic(audio->music_tracks[i]);
                audio->music_tracks[i] = NULL;
            }
        }
        
        // Free menu music
        if (audio->menu_music) {
            Mix_FreeMusic(audio->menu_music);
            audio->menu_music = NULL;
        }
        
        // Free arrays
        free(audio->music_tracks);
        free(audio->music_filenames);
        free(audio->gameplay_playlist);
        
        // Close audio
        Mix_CloseAudio();
        audio->initialized = false;
    }
}

// Load textures for billboarded vegetation
bool loadVegetationTextures() {
    const char* texture_files[] = {
        VEGETATION_TEXTURE_GRASS,
        VEGETATION_TEXTURE_GRASS,
        VEGETATION_TEXTURE_GRASS,
        VEGETATION_TEXTURE_GRASS,
        VEGETATION_TEXTURE_TREE1
    };
    
    for (int i = 0; i < 5; i++) {
        vegetation_textures[i] = loadTexture(texture_files[i]);
        if (vegetation_textures[i] == 0) {
            printf("Failed to load vegetation texture: %s\n", texture_files[i]);
            return false;
        }
    }
    
    return true;
}

// Create random vegetation
void createVegetation(int count, float terrain_size) {
    // Free previous vegetation if any
    if (vegetation != NULL) {
        free(vegetation);
    }
    
    // Allocate memory for vegetation
    vegetation_count = count;
    vegetation = (Vegetation*)malloc(sizeof(Vegetation) * vegetation_count);
    
    // Create random vegetation
    float half_size = terrain_size / 2.0f;
    for (int i = 0; i < vegetation_count; i++) {
        vegetation[i].x = ((float)rand() / RAND_MAX) * terrain_size - half_size;
        vegetation[i].z = ((float)rand() / RAND_MAX) * terrain_size - half_size;
        
        // Y position will be set based on terrain height
        vegetation[i].y = -1.5f; // Default ground level
        
        // Randomize texture (more trees than grass)
        int r = rand() % 100;
        if (r < 30) {
            vegetation[i].texture_index = 0; // 30% chance for grass
            vegetation[i].width = 1.0f + ((float)rand() / RAND_MAX) * 0.5f;
            vegetation[i].height = 0.5f + ((float)rand() / RAND_MAX) * 0.3f;
        } else {
            vegetation[i].texture_index = 1 + (rand() % 4); // 70% chance for trees (4 types)
            vegetation[i].width = 2.0f + ((float)rand() / RAND_MAX) * 1.5f;
            vegetation[i].height = 3.0f + ((float)rand() / RAND_MAX) * 2.0f;
        }
        
        vegetation[i].active = true;
    }
}

// Draw a billboard that always faces the camera
void drawBillboard(float x, float y, float z, float width, float height, GLuint texture) {
    // Get the modelview matrix
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    // Extract the right and up vectors from the modelview matrix
    float right_x = modelview[0];
    float right_y = modelview[4];
    float right_z = modelview[8];
    
    float up_x = modelview[1];
    float up_y = modelview[5];
    float up_z = modelview[9];
    
    // Calculate the four corners of the billboard
    float half_width = width / 2.0f;
    
    // Enable texturing and blending for transparent billboards
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Draw the billboard as a single quad
    glBegin(GL_QUADS);
    
    // Bottom left
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(
        x - right_x * half_width,
        y,
        z - right_z * half_width
    );
    
    // Bottom right
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(
        x + right_x * half_width,
        y,
        z + right_z * half_width
    );
    
    // Top right
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(
        x + right_x * half_width,
        y + height,
        z + right_z * half_width
    );
    
    // Top left
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(
        x - right_x * half_width,
        y + height,
        z - right_z * half_width
    );
    
    glEnd();
    
    // Disable texturing and blending
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

// Render all vegetation
void renderVegetation() {
    // Turn off lighting for billboards
    glDisable(GL_LIGHTING);
    
    // Render each piece of vegetation as a billboard
    for (int i = 0; i < vegetation_count; i++) {
        if (vegetation[i].active) {
            drawBillboard(
                vegetation[i].x, 
                vegetation[i].y, 
                vegetation[i].z, 
                vegetation[i].width, 
                vegetation[i].height, 
                vegetation_textures[vegetation[i].texture_index]
            );
        }
    }
    
    // Turn lighting back on
    glEnable(GL_LIGHTING);
}

// Initialize the game state and resources
bool initGame(GameState* game) {
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
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
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
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Create OpenGL context
    game->gl_context = SDL_GL_CreateContext(game->window);
    if (game->gl_context == NULL) {
        printf("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
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
        printf("Failed to load vegetation textures\n");
        return false;
    }
    
    // Create vegetation
    createVegetation(TERRAIN_MAX_FEATURES, TERRAIN_TILE_SIZE);
    
    // Initialize day-night cycle
    initEnvironment();
    setBackgroundColor(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);
    
    // Initialize UI system with smaller font size for game UI
    if (!initUI(&game->game_ui, GAME_FONT_FILE, GAME_UI_FONT_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        printf("Failed to initialize game UI system\n");
        return false;
    } else {
        printf("Game UI system initialized successfully with font: %s (size %d)\n", 
               GAME_FONT_FILE, GAME_UI_FONT_SIZE);
    }
    
    // Initialize UI system with larger font size for menu UI
    if (!initUI(&game->menu_ui, GAME_FONT_FILE, MENU_FONT_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        printf("Failed to initialize menu UI system\n");
        cleanupUI(&game->game_ui);
        return false;
    } else {
        printf("Menu UI system initialized successfully with font: %s (size %d)\n", 
               GAME_FONT_FILE, MENU_FONT_SIZE);
    }
    
    // Initialize audio system
    if (!initAudio(&game->audio)) {
        printf("Failed to initialize audio system\n");
        return false;
    } else {
        printf("Audio system initialized successfully\n");
    }
    
    // Create UI elements
    SDL_Color primary_color = {UI_PRIMARY_COLOR_R, UI_PRIMARY_COLOR_G, UI_PRIMARY_COLOR_B, UI_PRIMARY_COLOR_A};
    SDL_Color secondary_color = {UI_SECONDARY_COLOR_R, UI_SECONDARY_COLOR_G, UI_SECONDARY_COLOR_B, UI_PRIMARY_COLOR_A};
    SDL_Color accent_color = {UI_ACCENT_COLOR_R, UI_ACCENT_COLOR_G, UI_ACCENT_COLOR_B, UI_PRIMARY_COLOR_A};
    
    // Create FPS text
    game->fps_text_id = createTextElement(&game->game_ui, "FPS: 0", WINDOW_WIDTH - 20, 20, primary_color, TEXT_ALIGN_RIGHT);
    
    // Hide game UI initially (will be shown when game starts)
    setElementVisibility(&game->game_ui, game->fps_text_id, false);
    
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
                                              WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2, 
                                              primary_color, TEXT_ALIGN_RIGHT);
    
    game->settings_items[1] = createTextElement(&game->menu_ui, GAME_SETTINGS_SOUND, 
                                              WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 60, 
                                              primary_color, TEXT_ALIGN_RIGHT);
    
    // Create settings values (initially hidden)
    game->settings_values[0] = createTextElement(&game->menu_ui, GAME_SETTINGS_HIGH, 
                                               WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2, 
                                               primary_color, TEXT_ALIGN_LEFT);
    
    game->settings_values[1] = createTextElement(&game->menu_ui, "ON", 
                                               WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2 + 60, 
                                               primary_color, TEXT_ALIGN_LEFT);
    
    // Hide settings menu initially
    for (int i = 0; i < 2; i++) {
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
    SDL_Color accent_color = {UI_ACCENT_COLOR_R, UI_ACCENT_COLOR_G, UI_ACCENT_COLOR_B, UI_ACCENT_COLOR_A};
    
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
        
        // Hide settings menu
        for (int i = 0; i < 2; i++) {
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
        
        // Show settings items
        for (int i = 0; i < 2; i++) {
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
    }
    else if (game->menu_state == MENU_NONE) {
        // Game is running, hide all menu elements
        setElementVisibility(&game->menu_ui, game->menu_title_id, false);
        
        for (int i = 0; i < game->menu_item_count; i++) {
            setElementVisibility(&game->menu_ui, game->menu_items[i], false);
        }
        
        for (int i = 0; i < 2; i++) {
            setElementVisibility(&game->menu_ui, game->settings_items[i], false);
            setElementVisibility(&game->menu_ui, game->settings_values[i], false);
        }
    }
    
    // Show/hide game UI based on whether the game is running
    bool show_game_ui = (game->menu_state == MENU_NONE && game->game_started);
    setElementVisibility(&game->game_ui, game->fps_text_id, show_game_ui);
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
                    game->selected_menu_item = 1; // Settings has 2 items (0 and 1)
                }
                updateMenuUI(game);
                break;
                
            case SDLK_DOWN:
                // Move selection down in the settings menu
                game->selected_menu_item++;
                if (game->selected_menu_item > 1) {
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
    static Uint32 last_update = 0;
    Uint32 current_time = SDL_GetTicks();
    
    if (current_time - last_update >= 1000) {
        snprintf(buffer, sizeof(buffer), "FPS: %d", game->frame_count);
        updateTextElement(&game->game_ui, game->fps_text_id, buffer);
        
        last_update = current_time;
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
        
        // Update day-night cycle
        updateDayNightCycle(delta_time);
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
    // Apply player pitch rotation
    glRotatef(game->player.pitch, 1.0f, 0.0f, 0.0f);
    
    // Apply player yaw rotation
    glRotatef(game->player.yaw, 0.0f, 1.0f, 0.0f);
    
    // Translate to negative of player position
    glTranslatef(-game->player.position_x, 
                -game->player.position_y, 
                -game->player.position_z);
    
    // Setup lighting based on time of day
    setupLightingForTimeOfDay(current_tod);
    
    // Render terrain
    renderTerrain(game->terrain);
    
    // Render vegetation
    renderVegetation();
    
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
    Uint32 current_ticks = SDL_GetTicks();
    if (current_ticks - game->fps_last_time >= 1000) {
        printf("FPS: %d\n", game->frame_count);
        game->frame_count = 0;
        game->fps_last_time = current_ticks;
    }
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
    if (vegetation != NULL) {
        free(vegetation);
    }
    
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