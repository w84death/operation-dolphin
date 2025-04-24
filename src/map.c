#include "../include/map.h"
#include "../include/log.h"
#include "../include/model.h"
#include <math.h>

// Initialize the map view
void initializeMap(MapView* map_view) {
    // Set initial values
    map_view->active = false;
    map_view->scale = 2.0f;  // Increased scale for better visibility on the larger map
    map_view->width = MAP_WIDTH;
    map_view->height = MAP_HEIGHT;
    map_view->position_x = MAP_POSITION_X;
    map_view->position_y = MAP_POSITION_Y;
    
    // Load the map background texture using the existing function
    map_view->texture_id = loadTextureFromFile("textures/ui/map.tga");
    if (map_view->texture_id == 0) {
        logError("Failed to load map texture");
        return;
    }
    
    logInfo("Map view initialized with dimensions %dx%d", map_view->width, map_view->height);
}

// Toggle map visibility
void toggleMapView(MapView* map_view) {
    map_view->active = !map_view->active;
    logInfo("Map view toggled: %s", map_view->active ? "ON" : "OFF");
}

// Render the map and its elements
void renderMapView(MapView* map_view, Player* player, Wall* wall, 
                  StaticElement* static_elements, int static_element_count,
                  Animal* animals, int animal_count) {
    if (!map_view->active) {
        return; // Don't render if not active
    }
    
    // Save current matrices and set up orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1); // Adjust based on your window size
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Enable texturing and alpha blending
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw map background
    glBindTexture(GL_TEXTURE_2D, map_view->texture_id);
    glColor4f(1.0f, 1.0f, 1.0f, 0.92f); // More opaque background
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(map_view->position_x, map_view->position_y);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(map_view->position_x + map_view->width, map_view->position_y);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(map_view->position_x + map_view->width, map_view->position_y + map_view->height);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(map_view->position_x, map_view->position_y + map_view->height);
    glEnd();
    
    // Calculate map center and scaling - adjusted for the larger map
    int center_x = map_view->position_x + map_view->width / 2;
    int center_y = map_view->position_y + map_view->height / 2;
    float map_scale = map_view->scale * (map_view->width / 700.0f); // Increased scale for better visibility
    
    // Draw walls/boundaries using the terrain size
    if (wall) {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f); // Brighter walls for better visibility
        glLineWidth(3.0f); // Thicker lines for walls
        
        // Calculate wall dimensions based on terrain size and inset
        float terrainSize = TERRAIN_TILE_SIZE * TERRAIN_TILES_COUNT;
        float halfSize = terrainSize / 2.0f;
        float wallStart = -halfSize + wall->inset;
        float wallEnd = halfSize - wall->inset;
        
        // Draw the wall as a rectangle
        glBegin(GL_LINE_LOOP);
        // North wall
        float map_x1 = center_x + (wallStart - player->position_x) * map_scale;
        float map_y1 = center_y + (wallEnd - player->position_z) * map_scale;
        glVertex2f(map_x1, map_y1);
        
        // East wall
        float map_x2 = center_x + (wallEnd - player->position_x) * map_scale;
        float map_y2 = center_y + (wallEnd - player->position_z) * map_scale;
        glVertex2f(map_x2, map_y2);
        
        // South wall
        float map_x3 = center_x + (wallEnd - player->position_x) * map_scale;
        float map_y3 = center_y + (wallStart - player->position_z) * map_scale;
        glVertex2f(map_x3, map_y3);
        
        // West wall
        float map_x4 = center_x + (wallStart - player->position_x) * map_scale;
        float map_y4 = center_y + (wallStart - player->position_z) * map_scale;
        glVertex2f(map_x4, map_y4);
        
        glEnd();
        glLineWidth(1.0f); // Reset line width
    }
    
    // Draw static elements (infrastructure)
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.7f, 0.7f, 0.7f, 0.95f); // Brighter gray for static elements
    
    for (int i = 0; i < static_element_count; i++) {
        StaticElement* element = &static_elements[i];
        // Only render active elements
        if (!element->active) continue;
        
        float map_x = center_x + (element->x - player->position_x) * map_scale;
        float map_y = center_y + (element->z - player->position_z) * map_scale;
        
        // Draw a square for each static element
        glBegin(GL_QUADS);
        glVertex2f(map_x - 8, map_y - 8);  // Larger squares
        glVertex2f(map_x + 8, map_y - 8);
        glVertex2f(map_x + 8, map_y + 8);
        glVertex2f(map_x - 8, map_y + 8);
        glEnd();
    }
    
    // Draw animals as bright green dots (much more visible now)
    glColor4f(MAP_ANIMAL_COLOR_R, MAP_ANIMAL_COLOR_G, MAP_ANIMAL_COLOR_B, MAP_ANIMAL_COLOR_A);
    
    for (int i = 0; i < animal_count; i++) {
        Animal* animal = &animals[i];
        
        // Only render active animals
        if (!animal->active) continue;
        
        float map_x = center_x + (animal->x - player->position_x) * map_scale;
        float map_y = center_y + (animal->z - player->position_z) * map_scale;
        
        // Draw a circle for each animal
        const int segments = 14;  // More segments for smoother circles
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(map_x, map_y); // Center of circle
        
        for (int j = 0; j <= segments; j++) {
            float angle = 2.0f * M_PI * j / segments;
            float x = map_x + cos(angle) * MAP_ANIMAL_DOT_SIZE;
            float y = map_y + sin(angle) * MAP_ANIMAL_DOT_SIZE;
            glVertex2f(x, y);
        }
        glEnd();
    }
    
    // Draw player position (always at center)
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f); // Red for player
    
    // Draw player as a circle with direction indicator
    const float player_size = 10.0f;  // Larger player marker
    const int segments = 16;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(center_x, center_y); // Center of circle
    
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = center_x + cos(angle) * player_size;
        float y = center_y + sin(angle) * player_size;
        glVertex2f(x, y);
    }
    glEnd();
    
    // Direction indicator (shows where player is facing)
    // FIX: Correct the angle calculation - in Operation Dolphin, yaw is in degrees and 0 is looking along -Z axis
    // We need to convert from game coordinates to screen coordinates
    // Game: yaw 0 = -Z, increases clockwise
    // Screen: 0 = right (+X), increases clockwise
    float yawRadians = (player->yaw - 90.0f) * M_PI / 180.0f;  // -90 degrees to align with screen coordinates
    
    glColor4f(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for direction
    glLineWidth(4.0f);  // Make the direction line thicker for visibility
    glBegin(GL_LINES);
    glVertex2f(center_x, center_y);
    float dir_x = center_x + cos(yawRadians) * player_size * 3.0f;  // Longer direction indicator
    float dir_y = center_y + sin(yawRadians) * player_size * 3.0f;
    glVertex2f(dir_x, dir_y);
    glEnd();
    glLineWidth(1.0f);  // Reset line width
    
    // Reset state
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    
    // Restore matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
}

// Clean up resources used by the map
void cleanupMapView(MapView* map_view) {
    if (map_view->texture_id != 0) {
        glDeleteTextures(1, &map_view->texture_id);
        map_view->texture_id = 0;
    }
    logInfo("Map view resources cleaned up");
}