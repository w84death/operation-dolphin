#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdbool.h>
#include "model.h"
#include "game.h"

// Simplified enemy system for the FPS game
// This is just a stub to make the code compile, since we're not using enemies in the FPS game

// Maximum number of enemies
#define MAX_ENEMIES 1

// Enemy state
typedef enum {
    ENEMY_INACTIVE,
    ENEMY_ACTIVE
} EnemyState;

// Enemy formation type (not used in FPS game but kept for compatibility)
typedef enum {
    FORMATION_LINE,
    FORMATION_V,
    FORMATION_INVERTED_V,
    FORMATION_SQUARE,
    FORMATION_DIAMOND,
    FORMATION_CROSS,
    FORMATION_ZIGZAG,
    FORMATION_RANDOM
} FormationType;

// Wave difficulty settings
typedef struct {
    int enemy_count;
    float speed_multiplier;
    float lifetime_factor;
} WaveDifficulty;

// Enemy structure
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    bool active;
    EnemyState state;
    int model_index;
    float timer;
    float lifetime;
    float formation_offset_x;
    float formation_offset_y;
    float dir_change_timer;
} Enemy;

// Forward declaration for the Terrain struct
typedef struct Terrain Terrain;

// Function declarations
void initEnemies(void);
void spawnEnemy(int idx, int model_index, float formation_offset_x, float formation_offset_y, float speed_multiplier);
void spawnFormation(FormationType formation, int count, float speed_multiplier, float lifetime_factor);
void updateEnemies(float delta, float player_x, float player_y, Terrain* terrain, float terrain_speed);
void renderEnemy(Enemy* e);
void checkCollisions(float player_x, float player_y, float player_z, Terrain* terrain, float terrain_speed, GameState* game);
int getActiveEnemyCount(void);
int getCurrentWave(void);
void cleanupEnemies(void);

#endif // ENEMIES_H