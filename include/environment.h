#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <GL/gl.h>
#include <string.h> // Add string.h for memcpy

// Time of day enumeration
typedef enum {
    TOD_DAY,
    TOD_EVENING,
    TOD_NIGHT,
    TOD_MORNING
} TimeOfDay;

// Function declarations for environment settings
void initEnvironment(void);  // Added missing function declaration
void setupFog(float fog_start, float fog_end, float r, float g, float b, float a);
void setupLighting(void);
void setBackgroundColor(float r, float g, float b, float a);

// Interpolation helper functions
float lerp(float a, float b, float t);
void interpolateColor(float result[4], float colorA[4], float colorB[4], float t);
void interpolatePosition(float result[4], float posA[4], float posB[4], float t);

// New functions for time-based lighting
void initDayNightCycle(void);
void updateDayNightCycle(float delta_time);
TimeOfDay getCurrentTimeOfDay(void);
float getPreciseTimeOfDay(void);
void setupLightingForTimeOfDay(TimeOfDay time_of_day);
void setupFogForTimeOfDay(TimeOfDay time_of_day, float fog_start, float fog_end);

// Structure for lighting settings
typedef struct {
    GLfloat position[4];
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
} LightSettings;

// Structure for material settings
typedef struct {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess[1];
    GLfloat emission[4]; // Added emission for emissive materials
} MaterialSettings;

#endif // ENVIRONMENT_H