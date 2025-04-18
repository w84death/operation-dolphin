#ifndef CONFIG_H
#define CONFIG_H

// Window settings
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "OPERATION DOLPHIN"
#define GAME_MENU_TITLE "- O P E R A T I O N - D O L P H I N -"
#define GAME_MENU_RESUME "RESUME GAME"
#define GAME_MENU_NEWGAME "NEW GAME"
#define GAME_MENU_SETTINGS "SETTINGS"
#define GAME_MENU_QUIT "QUIT"
#define GAME_SETTINGS_QUALITY "QUALITY:"
#define GAME_SETTINGS_SOUND "SOUND:"
#define GAME_SETTINGS_HIGH "HIGH"
#define GAME_SETTINGS_LOW "LOW"
#define GAME_SETTINGS_ON "ON"
#define GAME_SETTINGS_OFF "OFF"
#define GAME_SETTINGS_INVERT "INVERT Y AXIS:"

// Audio settings
#define AUDIO_FREQUENCY 44100
#define AUDIO_FORMAT MIX_DEFAULT_FORMAT
#define AUDIO_CHANNELS 2
#define AUDIO_CHUNKSIZE 2048
#define MUSIC_VOLUME 64  // 0-128 range for SDL_mixer

#define MENU_MUSIC_FILE "music/menu.mp3"
#define MUSIC_TRACK_1 "music/track1.mp3"
#define MUSIC_TRACK_2 "music/track2.mp3"
#define MUSIC_TRACK_3 "music/menu.mp3"
#define NUM_MUSIC_TRACKS 3
#define TRACK_SWITCH_TIME 120.0f  // Time in seconds before switching tracks

// Input settings
#define MOUSE_INVERT_Y_DEFAULT true  // Default setting for Y axis inversion

// Environment settings
#define TIME_OF_THE_DAY_START 0.3f // 0.0 to 1.0 representing a full day-night cycle
#define TIME_OF_THE_DAY_DURATION 180.0f  // Length of a full day in seconds
#define FOG_COLOR_R 0.12f
#define FOG_COLOR_G 0.25f
#define FOG_COLOR_B 0.15f
#define FOG_COLOR_A 1.0f
#define BG_COLOR_R FOG_COLOR_R
#define BG_COLOR_G FOG_COLOR_G
#define BG_COLOR_B FOG_COLOR_B
#define BG_COLOR_A 1.0f
#define FOG_START 15.0f
#define FOG_END 50.0f

// Camera settings
#define CAMERA_FOV 75.0f
#define CAMERA_NEAR 0.1f
#define CAMERA_FAR FOG_END + 10.0f

// Player settings
#define PLAYER_POSITION_X 0.0f
#define PLAYER_POSITION_Y 0.0f
#define PLAYER_POSITION_Z 0.0f
#define PLAYER_EYE_HEIGHT 1.7f
#define PLAYER_HEIGHT 1.8f
#define PLAYER_MOVEMENT_SPEED 5.0f
#define PLAYER_JUMP_VELOCITY 5.0f
#define PLAYER_GRAVITY 9.8f
#define PLAYER_WEAPON_MODEL "models/weapon.glb"

// Terrain settings
#define TERRAIN_POSITION_X 0.0f
#define TERRAIN_POSITION_Y -1.5f
#define TERRAIN_POSITION_Z 0.0f
#define TERRAIN_TILE_SIZE 50.0f
#define TERRAIN_HEIGHT_SCALE 5.0f
#define TERRAIN_MAX_FEATURES 200

// Billboard vegetation settings
#define VEGETATION_TEXTURE_GRASS "textures/foliage/small/grass1.tga"
#define VEGETATION_TEXTURE_TREE1 "textures/foliage/big/tree1.tga"
#define VEGETATION_DENSITY 0.8f  // Higher values = more vegetation

// Font settings
#define GAME_FONT_FILE "fonts/NovaSquare-Regular.ttf"
#define MENU_FONT_SIZE 40
#define GAME_UI_FONT_SIZE 20  // Smaller size for game UI elements

// UI Color settings
#define UI_PRIMARY_COLOR_R 25
#define UI_PRIMARY_COLOR_G 25
#define UI_PRIMARY_COLOR_B 25
#define UI_PRIMARY_COLOR_A 255

#define UI_SECONDARY_COLOR_R 50
#define UI_SECONDARY_COLOR_G 220
#define UI_SECONDARY_COLOR_B 50
#define UI_SECONDARY_COLOR_A 255

#define UI_ACCENT_COLOR_R 220
#define UI_ACCENT_COLOR_G 220
#define UI_ACCENT_COLOR_B 50
#define UI_ACCENT_COLOR_A 255

#endif // CONFIG_H