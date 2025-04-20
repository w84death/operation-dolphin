#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "../include/audio.h"
#include "../include/log.h"

// Initialize audio system
bool initAudio(AudioSystem* audio) {
    // Initialize SDL_mixer
    if (Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNKSIZE) < 0) {
        logError("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
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
        logError("Failed to load menu music! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        logInfo("Menu music loaded successfully: %s\n", MENU_MUSIC_FILE);
    }
    
    // Load all music tracks
    bool loaded_at_least_one = false;
    for (int i = 0; i < audio->num_tracks; i++) {
        audio->music_tracks[i] = Mix_LoadMUS(audio->music_filenames[i]);
        if (!audio->music_tracks[i]) {
            logError("Failed to load music track %d (%s)! SDL_mixer Error: %s\n", 
                   i, audio->music_filenames[i], Mix_GetError());
        } else {
            logInfo("Music track %d loaded successfully: %s\n", i, audio->music_filenames[i]);
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
            logError("Failed to play music track %d! SDL_mixer Error: %s\n", 
                   track_index, Mix_GetError());
            audio->music_playing = false;
            return false;
        } else {
            audio->current_track = track_index;
            audio->music_playing = true;
            audio->in_menu_music = false;
            logInfo("Playing music track %d: %s\n", 
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
            logError("Failed to play menu music! SDL_mixer Error: %s\n", Mix_GetError());
            audio->music_playing = false;
            audio->in_menu_music = false;
            return false;
        } else {
            audio->music_playing = true;
            audio->in_menu_music = true;
            logInfo("Playing menu music\n");
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