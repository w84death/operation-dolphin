#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include "config.h"
#include <GL/gl.h>
#include <string.h> // Add string.h for memcpy

// Environment time of day and lighting
typedef struct {
    float timeOfDay;          // Value from 0.0f (midnight) to 1.0f (next midnight)
    float dayDuration;        // Duration of a full day cycle in seconds
    
    // Current light colors based on time of day
    float fogColor[4];
    float ambientLight[4];
    float diffuseLight[4];
    float specularLight[4];
    
    float fogStart;
    float fogEnd;
} Environment;

// Wall structure definition
typedef struct {
    GLuint texture;          // Fence texture
    float inset;             // Distance from terrain edge
    float height;            // Height of the wall
    float thickness;         // Thickness of the wall
    float segmentLength;     // Length of each fence segment
} Wall;

// Time of day enumeration
typedef enum {
    TOD_DAY,
    TOD_EVENING,
    TOD_NIGHT,
    TOD_MORNING
} TimeOfDay;

// Function declarations for environment settings
void initEnvironment(Environment *env);  // Updated function declaration
void updateEnvironment(Environment *env, float deltaTime);
void renderSky(Environment *env);
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

// Wall functions
void initWall(Wall *wall);
void renderWall(Wall *wall);
void renderWallSegment(Wall *wall, float length, float heightOffset);
int checkWallCollision(float x, float z, float radius, Wall *wall);  // Updated to include Wall pointer

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