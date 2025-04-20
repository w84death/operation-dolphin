#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>
#include "config.h"

// Forward declaration of AudioSystem
typedef struct AudioSystem AudioSystem;

// Audio structure for managing game audio
struct AudioSystem {
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
};

// Function declarations
bool initAudio(AudioSystem* audio);
bool playMusicTrack(AudioSystem* audio, int track_index);
bool playMenuMusic(AudioSystem* audio);
bool playNextGameplayTrack(AudioSystem* audio);
bool playRandomGameplayTrack(AudioSystem* audio);
void playBackgroundMusic(AudioSystem* audio);
void pauseBackgroundMusic(AudioSystem* audio);
void resumeBackgroundMusic(AudioSystem* audio);
void updateMusicSystem(AudioSystem* audio, float delta_time);
void cleanupAudio(AudioSystem* audio);

#endif // AUDIO_H