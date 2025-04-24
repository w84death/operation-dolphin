#include <GL/gl.h>
#include <math.h>
#include <string.h>
#include "../include/environment.h"
#include "../include/config.h"
#include "../include/model.h"
#include "../include/log.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>

// Global variables for day-night cycle
static float time_of_day = TIME_OF_THE_DAY_START;       // 0.0 to 1.0 representing a full day-night cycle
static float day_length = TIME_OF_THE_DAY_DURATION;      // Length of a full day in seconds
static TimeOfDay current_time_of_day = TOD_DAY;

// Initialize the environment
void initEnvironment(Environment *env) {
    // Initialize day-night cycle
    initDayNightCycle();
    
    // Setup initial lighting
    setupLighting();
    
    // Setup initial fog
    setupFog(FOG_START, FOG_END, FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_COLOR_A);
    
    // Set background color to match fog
    setBackgroundColor(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);
    
    // Initialize environment properties if env is not NULL
    if (env) {
        env->timeOfDay = TIME_OF_THE_DAY_START;
        env->dayDuration = TIME_OF_THE_DAY_DURATION;
        
        // Initialize fog properties
        env->fogStart = FOG_START;
        env->fogEnd = FOG_END;
        env->fogColor[0] = FOG_COLOR_R;
        env->fogColor[1] = FOG_COLOR_G;
        env->fogColor[2] = FOG_COLOR_B;
        env->fogColor[3] = FOG_COLOR_A;
        
        // Initialize light colors
        // We'll set these with the current time of day settings
        TimeOfDay current_tod = getCurrentTimeOfDay();
        setupLightingForTimeOfDay(current_tod);
    }
    
    printf("Environment initialized with day-night cycle\n");
}

// Helper function to linearly interpolate between two values
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Helper function to interpolate between colors
void interpolateColor(float result[4], float colorA[4], float colorB[4], float t) {
    result[0] = lerp(colorA[0], colorB[0], t);
    result[1] = lerp(colorA[1], colorB[1], t);
    result[2] = lerp(colorA[2], colorB[2], t);
    result[3] = lerp(colorA[3], colorB[3], t);
}

// Helper function to interpolate between positions
void interpolatePosition(float result[4], float posA[4], float posB[4], float t) {
    result[0] = lerp(posA[0], posB[0], t);
    result[1] = lerp(posA[1], posB[1], t);
    result[2] = lerp(posA[2], posB[2], t);
    result[3] = posA[3]; // Don't interpolate the last component (directional vs point light)
}

// Setup fog effect with specified parameters
void setupFog(float fog_start, float fog_end, float r, float g, float b, float a) {
    GLfloat fog_color[] = {r, g, b, a};
    
    // Enable fog
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fog_color);
    glFogf(GL_FOG_START, fog_start);
    glFogf(GL_FOG_END, fog_end);
}

// Setup lighting with default values
void setupLighting(void) {
    // Default light settings
    LightSettings light = {
        .position = {4.0f, 6.0f, 12.0f, 0.0f},
        .ambient = {0.6f, 0.5f, 0.3f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f, 1.0f}
    };
    
    // Default material settings
    MaterialSettings material = {
        .ambient = {0.5f, 0.5f, 0.5f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {0.8f, 0.8f, 0.8f, 1.0f},
        .shininess = {0.0f}
    };
    
    // Setup light
    glLightfv(GL_LIGHT0, GL_POSITION, light.position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light.ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light.diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light.specular);
    
    // Setup material
    glMaterialfv(GL_FRONT, GL_AMBIENT, material.ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material.shininess);
    
    // Enable lighting and first light
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Enable texture modulation with lighting
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Set background color
void setBackgroundColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

// Initialize day-night cycle
void initDayNightCycle(void) {
    time_of_day = 0.0f;
    current_time_of_day = TOD_DAY;
}

// Update day-night cycle based on elapsed time
void updateDayNightCycle(float delta_time) {
    // Progress time of day
    time_of_day += delta_time / day_length;
    if (time_of_day >= 1.0f) {
        time_of_day -= 1.0f;
    }
    
    // Determine current time of day period
    // New distribution: 40% day (0.0-0.4), 10% evening (0.4-0.5), 40% night (0.5-0.9), 10% morning (0.9-1.0)
    if (time_of_day < 0.4f) {
        current_time_of_day = TOD_DAY;
    } else if (time_of_day < 0.5f) {
        current_time_of_day = TOD_EVENING;
    } else if (time_of_day < 0.9f) {
        current_time_of_day = TOD_NIGHT;
    } else {
        current_time_of_day = TOD_MORNING; // We'll add this new state
    }
}

// Get current time of day
TimeOfDay getCurrentTimeOfDay(void) {
    return current_time_of_day;
}

// Get precise time of day (0.0 to 1.0)
float getPreciseTimeOfDay(void) {
    return time_of_day;
}

// Setup fog based on precise time of day
void setupFogForTimeOfDay(TimeOfDay time_period, float fog_start, float fog_end) {
    GLfloat fog_color[4];
    
    // Define the base colors for each time period using config values
    GLfloat day_color[4] = {DAY_FOG_COLOR_R, DAY_FOG_COLOR_G, DAY_FOG_COLOR_B, DAY_FOG_COLOR_A};
    GLfloat evening_color[4] = {EVENING_FOG_COLOR_R, EVENING_FOG_COLOR_G, EVENING_FOG_COLOR_B, EVENING_FOG_COLOR_A};
    GLfloat night_color[4] = {NIGHT_FOG_COLOR_R, NIGHT_FOG_COLOR_G, NIGHT_FOG_COLOR_B, NIGHT_FOG_COLOR_A};
    GLfloat morning_color[4] = {MORNING_FOG_COLOR_R, MORNING_FOG_COLOR_G, MORNING_FOG_COLOR_B, MORNING_FOG_COLOR_A};
    
    // Get the precise time (0.0 to 1.0)
    float precise_time = getPreciseTimeOfDay();
    
    // New distribution: 40% day (0.0-0.4), 10% evening (0.4-0.5), 40% night (0.5-0.9), 10% morning (0.9-1.0)
    if (precise_time < 0.39f) {
        // Day time
        memcpy(fog_color, day_color, sizeof(day_color));
    } 
    else if (precise_time < 0.5f) {
        // Day to evening transition
        float t = (precise_time - 0.39f) / 0.11f; // Normalize to 0.0-1.0 range
        interpolateColor(fog_color, day_color, evening_color, t);
    }
    else if (precise_time < 0.52f) {
        // Evening to night transition
        float t = (precise_time - 0.5f) / 0.02f; // Quick transition, normalized to 0.0-1.0
        interpolateColor(fog_color, evening_color, night_color, t);
    }
    else if (precise_time < 0.9f) {
        // Night time
        memcpy(fog_color, night_color, sizeof(night_color));
    }
    else if (precise_time < 0.98f) {
        // Night to morning transition
        float t = (precise_time - 0.9f) / 0.08f; // Normalize to 0.0-1.0 range
        interpolateColor(fog_color, night_color, morning_color, t);
    }
    else {
        // Morning to day transition
        float t = (precise_time - 0.98f) / 0.02f; // Quick transition, normalized to 0.0-1.0
        interpolateColor(fog_color, morning_color, day_color, t);
    }
    
    // Set background color to match fog
    setBackgroundColor(fog_color[0], fog_color[1], fog_color[2], fog_color[3]);
    
    // Setup fog with the interpolated color
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fog_color);
    glFogf(GL_FOG_START, fog_start);
    glFogf(GL_FOG_END, fog_end);
}

// Setup lighting based on precise time of day
void setupLightingForTimeOfDay(TimeOfDay time_period) {
    // Define lighting settings for each time period using config values
    LightSettings day_light = {
        .position = {4.0f, 6.0f, 12.0f, 0.0f},
        .ambient = {DAY_AMBIENT_R, DAY_AMBIENT_G, DAY_AMBIENT_B, 1.0f},
        .diffuse = {DAY_DIFFUSE_R, DAY_DIFFUSE_G, DAY_DIFFUSE_B, 1.0f},
        .specular = {DAY_SPECULAR_R, DAY_SPECULAR_G, DAY_SPECULAR_B, 1.0f}
    };
    
    LightSettings evening_light = {
        .position = {12.0f, 0.0f, 2.0f, 0.0f},
        .ambient = {EVENING_AMBIENT_R, EVENING_AMBIENT_G, EVENING_AMBIENT_B, 1.0f},
        .diffuse = {EVENING_DIFFUSE_R, EVENING_DIFFUSE_G, EVENING_DIFFUSE_B, 1.0f},
        .specular = {EVENING_SPECULAR_R, EVENING_SPECULAR_G, EVENING_SPECULAR_B, 1.0f}
    };
    
    LightSettings night_light = {
        .position = {-2.0f, 4.0f, -6.0f, 0.0f},
        .ambient = {NIGHT_AMBIENT_R, NIGHT_AMBIENT_G, NIGHT_AMBIENT_B, 1.0f},
        .diffuse = {NIGHT_DIFFUSE_R, NIGHT_DIFFUSE_G, NIGHT_DIFFUSE_B, 1.0f},
        .specular = {NIGHT_SPECULAR_R, NIGHT_SPECULAR_G, NIGHT_SPECULAR_B, 1.0f}
    };
    
    LightSettings morning_light = {
        .position = {-4.0f, 1.0f, 10.0f, 0.0f},
        .ambient = {MORNING_AMBIENT_R, MORNING_AMBIENT_G, MORNING_AMBIENT_B, 1.0f},
        .diffuse = {MORNING_DIFFUSE_R, MORNING_DIFFUSE_G, MORNING_DIFFUSE_B, 1.0f},
        .specular = {MORNING_SPECULAR_R, MORNING_SPECULAR_G, MORNING_SPECULAR_B, 1.0f}
    };
    
    // Material settings for different times of day
    MaterialSettings day_material = {
        .ambient = {0.6f, 0.6f, 0.6f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {0.8f, 0.8f, 0.8f, 1.0f},
        .shininess = {30.0f},
        .emission = {0.0f, 0.0f, 0.0f, 1.0f}
    };
    
    MaterialSettings evening_material = {
        .ambient = {0.5f, 0.4f, 0.3f, 1.0f},
        .diffuse = {0.9f, 0.7f, 0.5f, 1.0f},
        .specular = {0.7f, 0.5f, 0.3f, 1.0f},
        .shininess = {20.0f},
        .emission = {0.1f, 0.05f, 0.0f, 1.0f}
    };
    
    MaterialSettings night_material = {
        .ambient = {0.2f, 0.2f, 0.3f, 1.0f},
        .diffuse = {0.4f, 0.4f, 0.6f, 1.0f},
        .specular = {0.3f, 0.3f, 0.5f, 1.0f},
        .shininess = {10.0f},
        .emission = {0.02f, 0.02f, 0.05f, 1.0f}
    };
    
    MaterialSettings morning_material = {
        .ambient = {0.4f, 0.4f, 0.5f, 1.0f},
        .diffuse = {0.7f, 0.7f, 0.9f, 1.0f},
        .specular = {0.5f, 0.5f, 0.7f, 1.0f},
        .shininess = {15.0f},
        .emission = {0.05f, 0.05f, 0.1f, 1.0f}
    };
    
    // Interpolated light and material settings
    LightSettings light;
    MaterialSettings material;
    
    // Get the precise time (0.0 to 1.0)
    float precise_time = getPreciseTimeOfDay();
    
    // New distribution: 40% day (0.0-0.4), 10% evening (0.4-0.5), 40% night (0.5-0.9), 10% morning (0.9-1.0)
    // With smooth transitions added between evening/night and morning/day
    if (precise_time < 0.39f) {
        // Day time - use day settings
        memcpy(light.position, day_light.position, sizeof(day_light.position));
        memcpy(light.ambient, day_light.ambient, sizeof(day_light.ambient));
        memcpy(light.diffuse, day_light.diffuse, sizeof(day_light.diffuse));
        memcpy(light.specular, day_light.specular, sizeof(day_light.specular));
        
        memcpy(material.ambient, day_material.ambient, sizeof(day_material.ambient));
        memcpy(material.diffuse, day_material.diffuse, sizeof(day_material.diffuse));
        memcpy(material.specular, day_material.specular, sizeof(day_material.specular));
        memcpy(material.shininess, day_material.shininess, sizeof(day_material.shininess));
        memcpy(material.emission, day_material.emission, sizeof(day_material.emission));
    } 
    else if (precise_time < 0.5f) {
        // Day to evening transition (0.39-0.5)
        float t = (precise_time - 0.39f) / 0.11f;
        interpolatePosition(light.position, day_light.position, evening_light.position, t);
        interpolateColor(light.ambient, day_light.ambient, evening_light.ambient, t);
        interpolateColor(light.diffuse, day_light.diffuse, evening_light.diffuse, t);
        interpolateColor(light.specular, day_light.specular, evening_light.specular, t);
        
        interpolateColor(material.ambient, day_material.ambient, evening_material.ambient, t);
        interpolateColor(material.diffuse, day_material.diffuse, evening_material.diffuse, t);
        interpolateColor(material.specular, day_material.specular, evening_material.specular, t);
        material.shininess[0] = lerp(day_material.shininess[0], evening_material.shininess[0], t);
        interpolateColor(material.emission, day_material.emission, evening_material.emission, t);
    }
    else if (precise_time < 0.52f) {
        // Evening to night transition (0.5-0.52)
        float t = (precise_time - 0.5f) / 0.02f;
        interpolatePosition(light.position, evening_light.position, night_light.position, t);
        interpolateColor(light.ambient, evening_light.ambient, night_light.ambient, t);
        interpolateColor(light.diffuse, evening_light.diffuse, night_light.diffuse, t);
        interpolateColor(light.specular, evening_light.specular, night_light.specular, t);
        
        interpolateColor(material.ambient, evening_material.ambient, night_material.ambient, t);
        interpolateColor(material.diffuse, evening_material.diffuse, night_material.diffuse, t);
        interpolateColor(material.specular, evening_material.specular, night_material.specular, t);
        material.shininess[0] = lerp(evening_material.shininess[0], night_material.shininess[0], t);
        interpolateColor(material.emission, evening_material.emission, night_material.emission, t);
    }
    else if (precise_time < 0.9f) {
        // Night time - use night settings
        memcpy(light.position, night_light.position, sizeof(night_light.position));
        memcpy(light.ambient, night_light.ambient, sizeof(night_light.ambient));
        memcpy(light.diffuse, night_light.diffuse, sizeof(night_light.diffuse));
        memcpy(light.specular, night_light.specular, sizeof(night_light.specular));
        
        memcpy(material.ambient, night_material.ambient, sizeof(night_material.ambient));
        memcpy(material.diffuse, night_material.diffuse, sizeof(night_material.diffuse));
        memcpy(material.specular, night_material.specular, sizeof(night_material.specular));
        memcpy(material.shininess, night_material.shininess, sizeof(night_material.shininess));
        memcpy(material.emission, night_material.emission, sizeof(night_material.emission));
    }
    else if (precise_time < 0.98f) {
        // Night to morning transition (0.9-0.98)
        float t = (precise_time - 0.9f) / 0.08f;
        interpolatePosition(light.position, night_light.position, morning_light.position, t);
        interpolateColor(light.ambient, night_light.ambient, morning_light.ambient, t);
        interpolateColor(light.diffuse, night_light.diffuse, morning_light.diffuse, t);
        interpolateColor(light.specular, night_light.specular, morning_light.specular, t);
        
        interpolateColor(material.ambient, night_material.ambient, morning_material.ambient, t);
        interpolateColor(material.diffuse, night_material.diffuse, morning_material.diffuse, t);
        interpolateColor(material.specular, night_material.specular, morning_material.specular, t);
        material.shininess[0] = lerp(night_material.shininess[0], morning_material.shininess[0], t);
        interpolateColor(material.emission, night_material.emission, morning_material.emission, t);
    }
    else {
        // Morning to day transition (0.98-1.0)
        float t = (precise_time - 0.98f) / 0.02f;
        interpolatePosition(light.position, morning_light.position, day_light.position, t);
        interpolateColor(light.ambient, morning_light.ambient, day_light.ambient, t);
        interpolateColor(light.diffuse, morning_light.diffuse, day_light.diffuse, t);
        interpolateColor(light.specular, morning_light.specular, day_light.specular, t);
        
        interpolateColor(material.ambient, morning_material.ambient, day_material.ambient, t);
        interpolateColor(material.diffuse, morning_material.diffuse, day_material.diffuse, t);
        interpolateColor(material.specular, morning_material.specular, day_material.specular, t);
        material.shininess[0] = lerp(morning_material.shininess[0], day_material.shininess[0], t);
        interpolateColor(material.emission, morning_material.emission, day_material.emission, t);
    }
    
    // Setup light
    glLightfv(GL_LIGHT0, GL_POSITION, light.position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light.ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light.diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light.specular);
    
    // Setup material
    glMaterialfv(GL_FRONT, GL_AMBIENT, material.ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material.shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, material.emission);
    
    // Enable lighting and first light
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Enable texture modulation with lighting
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Initialize the wall with configuration parameters
void initWall(Wall *wall) {
    wall->texture = loadTextureFromFile(WALL_TEXTURE);
    wall->inset = WALL_INSET;
    wall->height = WALL_HEIGHT;
    wall->thickness = WALL_THICKNESS;
    wall->segmentLength = WALL_SEGMENT_LENGTH;
    
    if (wall->texture == 0) {
        logError("Failed to load wall texture: %s", WALL_TEXTURE);
    } else {
        logInfo("Wall texture loaded successfully");
    }
}

// Render the wall around the terrain
void renderWall(Wall *wall) {
    // Calculate wall dimensions based on terrain size and inset
    float terrainSize = TERRAIN_TILE_SIZE;
    float halfSize = terrainSize / 2.0f;
    float wallStart = -halfSize + wall->inset;
    float wallEnd = halfSize - wall->inset;
    
    // Skip rendering if texture failed to load
    if (wall->texture == 0) {
        logError("Cannot render wall - texture not loaded");
        return;
    }
    
    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);
    
    // Bind the fence texture
    glBindTexture(GL_TEXTURE_2D, wall->texture);
    
    // Enable vertex and texture arrays
    glEnableClientState(GL_VERTEX_ARRAY);  // Added this line
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Set material properties for the wall
    float matAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    float matDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    
    // Render the wall segments
    // North side (positive Z)
    for (float x = wallStart; x < wallEnd; x += wall->segmentLength) {
        float segmentLength = fmin(wall->segmentLength, wallEnd - x);
        
        glPushMatrix();
        glTranslatef(x, 0, wallEnd);
        renderWallSegment(wall, segmentLength, 0);
        glPopMatrix();
    }
    
    // South side (negative Z)
    for (float x = wallStart; x < wallEnd; x += wall->segmentLength) {
        float segmentLength = fmin(wall->segmentLength, wallEnd - x);
        
        glPushMatrix();
        glTranslatef(x, 0, wallStart);
        glRotatef(180, 0, 1, 0);
        renderWallSegment(wall, segmentLength, 0);
        glPopMatrix();
    }
    
    // East side (positive X)
    for (float z = wallStart; z < wallEnd; z += wall->segmentLength) {
        float segmentLength = fmin(wall->segmentLength, wallEnd - z);
        
        glPushMatrix();
        glTranslatef(wallEnd, 0, z);
        glRotatef(90, 0, 1, 0);
        renderWallSegment(wall, segmentLength, 0);
        glPopMatrix();
    }
    
    // West side (negative X)
    for (float z = wallStart; z < wallEnd; z += wall->segmentLength) {
        float segmentLength = fmin(wall->segmentLength, wallEnd - z);
        
        glPushMatrix();
        glTranslatef(wallStart, 0, z);
        glRotatef(-90, 0, 1, 0);
        renderWallSegment(wall, segmentLength, 0);
        glPopMatrix();
    }
    
    // Disable client states
    glDisableClientState(GL_VERTEX_ARRAY);  // Added this line
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

// Helper function to render a single wall segment
void renderWallSegment(Wall *wall, float length, float heightOffset) {
    // Enable alpha blending for transparent parts of fence
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f); // Skip nearly transparent pixels
    
    // Better texture coordinates that repeat the fence texture properly
    float texCoords[] = {
        0.0f, 1.0f,       // Bottom left
        length / 2.0f, 1.0f,  // Bottom right (repeat texture every 2 units)
        length / 2.0f, 0.0f,  // Top right
        0.0f, 0.0f        // Top left
    };
    
    // Reversed vertex order to make the wall visible from inside the terrain
    float vertices[] = {
        // Original order was: BL, BR, TR, TL
        // Reversed to: BR, BL, TL, TR
        length, 0.0f + heightOffset, 0.0f,      // Bottom right
        0.0f, 0.0f + heightOffset, 0.0f,        // Bottom left
        0.0f, wall->height + heightOffset, 0.0f, // Top left
        length, wall->height + heightOffset, 0.0f // Top right
    };
    
    // Also reverse texture coordinates to match reversed vertices
    float reversedTexCoords[] = {
        length / 2.0f, 1.0f,  // Bottom right
        0.0f, 1.0f,       // Bottom left
        0.0f, 0.0f,       // Top left
        length / 2.0f, 0.0f   // Top right
    };
    
    glTexCoordPointer(2, GL_FLOAT, 0, reversedTexCoords);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_QUADS, 0, 4);
    
    // Disable alpha testing and blending when done
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}

// Check for collision with the wall
int checkWallCollision(float x, float z, float radius) {
    float terrainSize = TERRAIN_TILE_SIZE;
    float halfSize = terrainSize / 2.0f;
    float wallStart = -halfSize + WALL_INSET;
    float wallEnd = halfSize - WALL_INSET;
    
    // Add a small buffer to prevent getting too close to the wall
    float buffer = radius + 0.1f;
    
    // Check collision with each wall
    // North wall (positive Z)
    if (z + buffer > wallEnd && x >= wallStart - buffer && x <= wallEnd + buffer) {
        return 1;
    }
    
    // South wall (negative Z)
    if (z - buffer < wallStart && x >= wallStart - buffer && x <= wallEnd + buffer) {
        return 1;
    }
    
    // East wall (positive X)
    if (x + buffer > wallEnd && z >= wallStart - buffer && z <= wallEnd + buffer) {
        return 1;
    }
    
    // West wall (negative X)
    if (x - buffer < wallStart && z >= wallStart - buffer && z <= wallEnd + buffer) {
        return 1;
    }
    
    // No collision
    return 0;
}