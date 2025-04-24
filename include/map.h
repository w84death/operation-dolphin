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