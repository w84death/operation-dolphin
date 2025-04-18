#include <GL/gl.h>
#include <math.h>
#include <string.h>
#include "../include/environment.h"
#include "../include/config.h"

// Global variables for day-night cycle
static float time_of_day = TIME_OF_THE_DAY_START;       // 0.0 to 1.0 representing a full day-night cycle
static float day_length = TIME_OF_THE_DAY_DURATION;      // Length of a full day in seconds
static TimeOfDay current_time_of_day = TOD_DAY;

// Initialize the environment
void initEnvironment(void) {
    // Initialize day-night cycle
    initDayNightCycle();
    
    // Setup initial lighting
    setupLighting();
    
    // Setup initial fog
    setupFog(FOG_START, FOG_END, FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_COLOR_A);
    
    // Set background color to match fog
    setBackgroundColor(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);
    
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
    
    // Determine current time of day period (for compatibility with existing code)
    if (time_of_day < 0.33f) {
        current_time_of_day = TOD_DAY;
    } else if (time_of_day < 0.66f) {
        current_time_of_day = TOD_EVENING;
    } else {
        current_time_of_day = TOD_NIGHT;
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
    
    // Define the base colors for each time period
    GLfloat day_color[4] = {FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_COLOR_A};
    GLfloat evening_color[4] = {0.7f, 0.5f, 0.3f, FOG_COLOR_A};
    GLfloat night_color[4] = {0.05f, 0.05f, 0.15f, FOG_COLOR_A};
    
    // Get the precise time (0.0 to 1.0)
    float precise_time = getPreciseTimeOfDay();
    
    // Determine which color transition we're in and calculate interpolation factor
    if (precise_time < 0.33f) {
        // Transition from night to day (since we wrap around)
        float t = 0;
        if (precise_time > 0.0f) {
            t = precise_time / 0.33f; // 0.0 to 1.0 interpolation factor
        }
        
        // If precise_time is close to 0, we need to blend from night to day
        if (precise_time < 0.05f) {
            t = precise_time / 0.05f; // Faster transition at cycle boundary
            interpolateColor(fog_color, night_color, day_color, t);
        } else {
            // Otherwise we're firmly in day territory
            memcpy(fog_color, day_color, sizeof(day_color));
        }
    } 
    else if (precise_time < 0.66f) {
        // Day to evening transition
        float t = (precise_time - 0.33f) / (0.66f - 0.33f);
        interpolateColor(fog_color, day_color, evening_color, t);
    } 
    else {
        // Evening to night transition
        float t = (precise_time - 0.66f) / (1.0f - 0.66f);
        interpolateColor(fog_color, evening_color, night_color, t);
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
    // Define lighting settings for each time period
    LightSettings day_light = {
        .position = {4.0f, 6.0f, 12.0f, 0.0f},
        .ambient = {0.6f, 0.5f, 0.3f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f, 1.0f}
    };
    
    LightSettings evening_light = {
        .position = {12.0f, 0.0f, 2.0f, 0.0f},
        .ambient = {0.5f, 0.3f, 0.2f, 1.0f},
        .diffuse = {0.9f, 0.6f, 0.4f, 1.0f},
        .specular = {0.8f, 0.6f, 0.4f, 1.0f}
    };
    
    LightSettings night_light = {
        .position = {-2.0f, 4.0f, -6.0f, 0.0f},
        .ambient = {0.05f, 0.05f, 0.1f, 1.0f},
        .diffuse = {0.1f, 0.1f, 0.2f, 1.0f},
        .specular = {0.15f, 0.15f, 0.25f, 1.0f}
    };
    
    // Default material settings (same for all times)
    MaterialSettings material = {
        .ambient = {0.5f, 0.5f, 0.5f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {0.8f, 0.8f, 0.8f, 1.0f},
        .shininess = {0.0f},
        .emission = {0.0f, 0.0f, 0.0f, 1.0f}
    };
    
    // Get the precise time (0.0 to 1.0)
    float precise_time = getPreciseTimeOfDay();
    
    // Interpolated light settings
    LightSettings light;
    
    // Determine which transition we're in and calculate interpolation factor
    if (precise_time < 0.33f) {
        // Night to day transition (near cycle boundary)
        if (precise_time < 0.05f) {
            float t = precise_time / 0.05f; // Faster transition at cycle boundary
            interpolatePosition(light.position, night_light.position, day_light.position, t);
            interpolateColor(light.ambient, night_light.ambient, day_light.ambient, t);
            interpolateColor(light.diffuse, night_light.diffuse, day_light.diffuse, t);
            interpolateColor(light.specular, night_light.specular, day_light.specular, t);
        } else {
            // Regular day lighting
            memcpy(light.position, day_light.position, sizeof(day_light.position));
            memcpy(light.ambient, day_light.ambient, sizeof(day_light.ambient));
            memcpy(light.diffuse, day_light.diffuse, sizeof(day_light.diffuse));
            memcpy(light.specular, day_light.specular, sizeof(day_light.specular));
        }
    } 
    else if (precise_time < 0.66f) {
        // Day to evening transition
        float t = (precise_time - 0.33f) / (0.66f - 0.33f);
        interpolatePosition(light.position, day_light.position, evening_light.position, t);
        interpolateColor(light.ambient, day_light.ambient, evening_light.ambient, t);
        interpolateColor(light.diffuse, day_light.diffuse, evening_light.diffuse, t);
        interpolateColor(light.specular, day_light.specular, evening_light.specular, t);
    } 
    else {
        // Evening to night transition
        float t = (precise_time - 0.66f) / (1.0f - 0.66f);
        interpolatePosition(light.position, evening_light.position, night_light.position, t);
        interpolateColor(light.ambient, evening_light.ambient, night_light.ambient, t);
        interpolateColor(light.diffuse, evening_light.diffuse, night_light.diffuse, t);
        interpolateColor(light.specular, evening_light.specular, night_light.specular, t);
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