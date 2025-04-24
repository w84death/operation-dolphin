#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "player.h"
#include "environment.h"
#include "static_elements.h"
#include "animals.h"
#include "config.h"

// Map size constants - dramatically larger now
#define MAP_WIDTH 700
#define MAP_HEIGHT 700
#define MAP_POSITION_X 40
#define MAP_POSITION_Y 20
#define MAP_ANIMAL_DOT_SIZE 10.0f  // Larger dots for better visibility
#define MAP_ANIMAL_COLOR_R 0.0f
#define MAP_ANIMAL_COLOR_G 1.0f
#define MAP_ANIMAL_COLOR_B 0.2f
#define MAP_ANIMAL_COLOR_A 1.0f   // Full opacity for better visibility

// Map view state
typedef struct {
    bool active;           // Is map currently shown
    GLuint texture_id;     // Texture for map background
    float scale;           // Map zoom level
    int width;             // Map width in pixels
    int height;            // Map height in pixels
    int position_x;        // Position on screen
    int position_y;        // Position on screen
} MapView;

// Function declarations
void initializeMap(MapView* map_view);
void toggleMapView(MapView* map_view);
void renderMapView(MapView* map_view, Player* player, Wall* wall, StaticElement* static_elements, int static_element_count, Animal* animals, int animal_count);
void cleanupMapView(MapView* map_view);

#endif // MAP_H