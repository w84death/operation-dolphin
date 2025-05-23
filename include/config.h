#ifndef CONFIG_H
#define CONFIG_H

// Window settings
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "OPERATION DOLPHIN by P1X"
#define GAME_ENGINE_VERSION "2025-04-23 alpha6"
#define GAME_MENU_TITLE "OPERATION DOLPHIN"
#define GAME_MENU_RESUME "RESUME GAME"
#define GAME_MENU_NEWGAME "NEW GAME"
#define GAME_MENU_SETTINGS "SETTINGS"
#define GAME_MENU_QUIT "QUIT"
#define GAME_SETTINGS_QUALITY "QUALITY:"
#define GAME_SETTINGS_SOUND "MUSIC:"
#define GAME_SETTINGS_SFX "SFX:"
#define GAME_SETTINGS_HIGH "HIGH"
#define GAME_SETTINGS_LOW "LOW"
#define GAME_SETTINGS_ON "ON"
#define GAME_SETTINGS_OFF "OFF"
#define GAME_SETTINGS_INVERT "INVERT Y AXIS:"

// Static element settings
#define MAX_STATIC_ELEMENTS 100
#define STATIC_ELEMENT_DIRECTIONS 8
#define MAX_STATIC_ELEMENT_TYPES 8

// Audio settings
#define AUDIO_FREQUENCY 44100
#define AUDIO_FORMAT MIX_DEFAULT_FORMAT
#define AUDIO_CHANNELS 2
#define AUDIO_CHUNKSIZE 2048
#define MUSIC_VOLUME 64  // 0-128 range for SDL_mixer
#define SFX_VOLUME 64    // 0-128 range for SDL_mixer
#define AMBIENT_VOLUME 48 // Slightly lower volume for ambient sounds

#define MENU_MUSIC_FILE "music/menu.mp3"
#define MUSIC_TRACK_1 "music/track1.mp3"
#define MUSIC_TRACK_2 "music/track2.mp3"
#define MUSIC_TRACK_3 "music/menu.mp3"
#define AMBIENT_SOUND_FILE "audio/ambient.mp3"
#define NUM_MUSIC_TRACKS 3
#define TRACK_SWITCH_TIME 120.0f  // Time in seconds before switching tracks

// Input settings
#define MOUSE_INVERT_Y_DEFAULT true  // Default setting for Y axis inversion

// Environment settings
#define TIME_OF_THE_DAY_START 0.20f // 0.0 to 1.0 representing a full day-night cycle
#define TIME_OF_THE_DAY_DURATION 120.0f  // Length of a full day in seconds

// Day/night cycle colors
// DAY colors (40% of cycle: 0.0-0.4)
#define DAY_FOG_COLOR_R 0.45f
#define DAY_FOG_COLOR_G 0.52f
#define DAY_FOG_COLOR_B 0.47f
#define DAY_FOG_COLOR_A 0.5f
#define DAY_AMBIENT_R 0.6f
#define DAY_AMBIENT_G 0.5f
#define DAY_AMBIENT_B 0.3f
#define DAY_DIFFUSE_R 1.0f
#define DAY_DIFFUSE_G 1.0f
#define DAY_DIFFUSE_B 1.0f
#define DAY_SPECULAR_R 1.0f
#define DAY_SPECULAR_G 1.0f
#define DAY_SPECULAR_B 1.0f

// EVENING colors (10% of cycle: 0.4-0.5)
#define EVENING_FOG_COLOR_R 0.7f
#define EVENING_FOG_COLOR_G 0.5f
#define EVENING_FOG_COLOR_B 0.3f
#define EVENING_FOG_COLOR_A 0.5f
#define EVENING_AMBIENT_R 0.5f
#define EVENING_AMBIENT_G 0.3f
#define EVENING_AMBIENT_B 0.2f
#define EVENING_DIFFUSE_R 0.9f
#define EVENING_DIFFUSE_G 0.6f
#define EVENING_DIFFUSE_B 0.4f
#define EVENING_SPECULAR_R 0.8f
#define EVENING_SPECULAR_G 0.6f
#define EVENING_SPECULAR_B 0.4f

// NIGHT colors (40% of cycle: 0.5-0.9)
#define NIGHT_FOG_COLOR_R 0.05f
#define NIGHT_FOG_COLOR_G 0.05f
#define NIGHT_FOG_COLOR_B 0.15f
#define NIGHT_FOG_COLOR_A 0.5f
#define NIGHT_AMBIENT_R 0.05f
#define NIGHT_AMBIENT_G 0.05f
#define NIGHT_AMBIENT_B 0.1f
#define NIGHT_DIFFUSE_R 0.1f
#define NIGHT_DIFFUSE_G 0.1f
#define NIGHT_DIFFUSE_B 0.2f
#define NIGHT_SPECULAR_R 0.15f
#define NIGHT_SPECULAR_G 0.15f
#define NIGHT_SPECULAR_B 0.25f

// MORNING colors (10% of cycle: 0.9-1.0)
#define MORNING_FOG_COLOR_R 0.6f
#define MORNING_FOG_COLOR_G 0.6f
#define MORNING_FOG_COLOR_B 0.8f
#define MORNING_FOG_COLOR_A 0.5f
#define MORNING_AMBIENT_R 0.4f
#define MORNING_AMBIENT_G 0.4f
#define MORNING_AMBIENT_B 0.6f
#define MORNING_DIFFUSE_R 0.8f
#define MORNING_DIFFUSE_G 0.8f
#define MORNING_DIFFUSE_B 1.0f
#define MORNING_SPECULAR_R 0.6f
#define MORNING_SPECULAR_G 0.6f
#define MORNING_SPECULAR_B 0.8f

// Legacy fog color definitions (keeping for compatibility)
#define FOG_COLOR_R DAY_FOG_COLOR_R
#define FOG_COLOR_G DAY_FOG_COLOR_G
#define FOG_COLOR_B DAY_FOG_COLOR_B
#define FOG_COLOR_A DAY_FOG_COLOR_A
#define BG_COLOR_R FOG_COLOR_R
#define BG_COLOR_G FOG_COLOR_G
#define BG_COLOR_B FOG_COLOR_B
#define BG_COLOR_A 1.0f
#define FOG_START 2.0f
#define FOG_END 15.0f

// Camera settings
#define CAMERA_FOV 75.0f
#define CAMERA_NEAR 0.1f
#define CAMERA_FAR FOG_END + 10.0f

// Menu camera position (elevated position for menu view)
#define MENU_CAMERA_HEIGHT 4.5f
#define MENU_CAMERA_PITCH 5.0f  // Look down at terrain
#define MENU_CAMERA_ROTATION_SPEED 0.005f

// Player settings
#define PLAYER_MAXIMUM_VERTICAL_ROT 160.0f // Max vertical rotation in degrees
#define PLAYER_EYE_HEIGHT 1.7f
#define PLAYER_MOVEMENT_SPEED 5.0f
#define PLAYER_JUMP_VELOCITY 5.0f
#define PLAYER_GRAVITY 9.8f
#define PLAYER_HEIGHT 2.0f       // Player's full height
#define PLAYER_RADIUS 0.4f       // Player collision radius for wall detection
#define PLAYER_FRICTION 0.9f     // Friction coefficient for player movement (0.9f = 10% slowdown per frame)
#define PLAYER_MAX_SLOPE_CHANGE 0.75f // Maximum terrain slope the player can climb

// Weapon display settings
#define PLAYER_WEAPON_POSITION_X 0.5f     // X position of weapon in view
#define PLAYER_WEAPON_POSITION_Y -0.4f    // Y position of weapon in view
#define PLAYER_WEAPON_POSITION_Z -1.0f    // Z position (distance from camera)
#define PLAYER_WEAPON_ROTATION_X -5.0f    // X rotation of weapon
#define PLAYER_WEAPON_ROTATION_Y 5.0f     // Y rotation of weapon
#define PLAYER_WEAPON_SIZE 0.5f           // Size of weapon quad

// Weapon settings
#define PLAYER_WEAPON_TEXTURE_BASE "textures/weapons/weapon0_"
#define PLAYER_WEAPON_TEXTURE_0 "textures/weapons/weapon0_0.tga"
#define PLAYER_WEAPON_TEXTURE_1 "textures/weapons/weapon0_1.tga"
#define PLAYER_WEAPON_TEXTURE_2 "textures/weapons/weapon0_2.tga"
#define WEAPON_ANIM_FRAME_TIME 0.1f   // Time per animation frame in seconds
#define FOLIAGE_CUTTING_RANGE 2.0f    // How far player can reach to cut foliage
#define FOLIAGE_CUTTING_COOLDOWN 0.5f // Cooldown between cuts in seconds

// Terrain settings
#define TERRAIN_POSITION_X 0.0f
#define TERRAIN_POSITION_Y 0.0f
#define TERRAIN_POSITION_Z 0.0f
#define TERRAIN_TILE_SIZE 16.0f
#define TERRAIN_TILES_COUNT 8
#define TERRAIN_HEIGHT_SCALE 2.5f

// Billboard vegetation settings
#define TERRAIN_MAX_FEATURES 5000
#define VEGETATION_DENSITY_SMALL 8.0f  // Multiplier for small vegetation
#define VEGETATION_DENSITY_MEDIUM 2.0f  // Multiplier for medium vegetation
#define VEGETATION_DENSITY_BIG 0.75f  // Multiplier for big vegetation
#define FOLIAGE_DEFAULT_SEED 8086    // Default seed for vegetation generation


// Wall settings
#define WALL_TEXTURE "textures/infrastructure/fence1.tga"
#define WALL_HEIGHT 2.0f
#define WALL_THICKNESS 0.5f
#define WALL_INSET CAMERA_FAR
#define WALL_SEGMENT_LENGTH 2.0f  // Length of each fence segment


// Font settings
#define GAME_FONT_FILE "fonts/NovaSquare-Regular.ttf"
#define MENU_FONT_SIZE 40
#define VERSION_FONT_SIZE 16  // Half size font for version text
#define GAME_UI_FONT_SIZE 20  // Smaller size for game UI elements

// UI Color settings
#define UI_PRIMARY_COLOR_R 250
#define UI_PRIMARY_COLOR_G 250
#define UI_PRIMARY_COLOR_B 255
#define UI_PRIMARY_COLOR_A 255

#define UI_SECONDARY_COLOR_R 50
#define UI_SECONDARY_COLOR_G 220
#define UI_SECONDARY_COLOR_B 50
#define UI_SECONDARY_COLOR_A 255

#define UI_ACCENT_COLOR_R 220
#define UI_ACCENT_COLOR_G 220
#define UI_ACCENT_COLOR_B 50
#define UI_ACCENT_COLOR_A 255

#define UI_DIM_COLOR_R 5
#define UI_DIM_COLOR_G 5
#define UI_DIM_COLOR_B 5
#define UI_DIM_COLOR_A 96

// Menu position settings
#define MENU_TITLE_Y_POS (WINDOW_HEIGHT / 4)
#define MENU_FIRST_ITEM_Y_POS (WINDOW_HEIGHT / 2 - 30)
#define MENU_ITEM_Y_SPACING 40
#define MENU_SETTINGS_LABEL_X_OFFSET 50
#define MENU_SETTINGS_VALUE_X_OFFSET 50
#define MENU_SETTINGS_FIRST_ITEM_Y_POS (WINDOW_HEIGHT / 2 - 60)

// Additional menu position constants
#define MENU_TITLE_X_POS (WINDOW_WIDTH / 2)
#define MENU_ITEM_X_POS (WINDOW_WIDTH / 2)
#define MENU_SETTINGS_LABEL_X_POS (WINDOW_WIDTH / 2 - MENU_SETTINGS_LABEL_X_OFFSET)
#define MENU_SETTINGS_VALUE_X_POS (WINDOW_WIDTH / 2 + MENU_SETTINGS_VALUE_X_OFFSET)

// Compass UI settings
#define COMPASS_WIDTH 300
#define COMPASS_HEIGHT 30
#define COMPASS_Y_POSITION 30
#define COMPASS_CARDINAL_WIDTH 20
#define COMPASS_LINE_HEIGHT 10

// Direction labels
#define COMPASS_NORTH "N"
#define COMPASS_EAST "E"
#define COMPASS_SOUTH "S"
#define COMPASS_WEST "W"

// Item settings
#define MAX_ITEMS 32
#define ITEM_INTERACTION_RANGE 1.0f
#define ITEM_SPAWN_MULTIPLIER 2.0f


// Map size constants - dramatically larger now
#define MAP_WIDTH 700
#define MAP_HEIGHT 700
#define MAP_POSITION_X 40
#define MAP_POSITION_Y 20
#define MAP_ANIMAL_DOT_SIZE 1.0f  // Larger dots for better visibility
#define MAP_STATIC_ELEMENT_DOT_SIZE 1.2f  // Slightly larger dots for static elements
#define MAP_ANIMAL_COLOR_R 0.0f
#define MAP_ANIMAL_COLOR_G 1.0f
#define MAP_ANIMAL_COLOR_B 0.2f
#define MAP_ANIMAL_COLOR_A 1.0f   // Full opacity for better visibility

// Constants for animal textures and rendering
#define MAX_ANIMAL_SPECIES 16
#define MAX_ANIMAL_COUNT 500

// Animal movement constants
#define ANIMAL_MIN_IDLE_TIME 1.0f     // Minimum idle time in seconds
#define ANIMAL_MAX_IDLE_TIME 5.0f     // Maximum idle time in seconds
#define ANIMAL_MIN_WALK_TIME 2.0f     // Minimum walking time in seconds
#define ANIMAL_MAX_WALK_TIME 8.0f     // Maximum walking time in seconds
#define ANIMAL_WANDER_RADIUS 15.0f    // Maximum distance animals can wander from spawn point

// Flying animal constants
#define FLYING_MIN_HEIGHT 6.0f        // Minimum flying height above terrain
#define FLYING_MAX_HEIGHT 12.0f       // Maximum flying height above terrain
#define FLYING_VERTICAL_SPEED 1.5f    // Vertical movement speed
#define FLYING_MIN_HEIGHT_TIME 5.0f   // Minimum time before changing height
#define FLYING_MAX_HEIGHT_TIME 12.0f  // Maximum time before changing height

#endif // CONFIG_H