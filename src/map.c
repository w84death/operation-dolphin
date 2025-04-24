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
    
    // Debug logging to check what we're trying to render
    logInfo("Map rendering - Animal count: %d, Static element count: %d", animal_count, static_element_count);

    // Count active entities
    int active_animals = 0;
    int active_elements = 0;
    
    for (int i = 0; i < animal_count; i++) {
        if (animals[i].active) active_animals++;
    }
    
    for (int i = 0; i < static_element_count; i++) {
        if (static_elements[i].active) active_elements++;
    }
    
    logInfo("Map rendering - Active animals: %d, Active static elements: %d", active_animals, active_elements);
    
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
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White walls for better visibility
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
    
    // Draw static elements as white triangles
    glDisable(GL_TEXTURE_2D);
    
    for (int i = 0; i < static_element_count; i++) {
        StaticElement* element = &static_elements[i];
        // Only render active elements
        if (!element->active) continue;
        
        float map_x = center_x + (element->x - player->position_x) * map_scale;
        float map_y = center_y + (element->z - player->position_z) * map_scale;
        float triangle_size = 12.0f;  // Size of triangles for static elements
        
        // All elements are white now
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Draw a triangle for each static element (pointing upward)
        glBegin(GL_TRIANGLES);
        glVertex2f(map_x, map_y - triangle_size);  // Bottom left
        glVertex2f(map_x + triangle_size, map_y - triangle_size);  // Bottom right
        glVertex2f(map_x + triangle_size/2, map_y);  // Top
        glEnd();
        
        // Draw outline for better visibility
        glLineWidth(1.5f);
        glColor4f(0.3f, 0.3f, 0.3f, 1.0f);  // Dark outline
        glBegin(GL_LINE_LOOP);
        glVertex2f(map_x, map_y - triangle_size);
        glVertex2f(map_x + triangle_size, map_y - triangle_size);
        glVertex2f(map_x + triangle_size/2, map_y);
        glEnd();
    }
    
    // Draw animals as white circles
    for (int i = 0; i < animal_count; i++) {
        Animal* animal = &animals[i];
        
        // Only render active animals
        if (!animal->active) continue;
        
        float map_x = center_x + (animal->x - player->position_x) * map_scale;
        float map_y = center_y + (animal->z - player->position_z) * map_scale;
        
        // All animals are white now
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Draw a circle for each animal
        const int segments = 14;
        float circle_size = MAP_ANIMAL_DOT_SIZE * 4.0f; // Make circles more visible
        
        // Fill circle
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(map_x, map_y); // Center of circle
        
        for (int j = 0; j <= segments; j++) {
            float angle = 2.0f * M_PI * j / segments;
            float x = map_x + cos(angle) * circle_size;
            float y = map_y + sin(angle) * circle_size;
            glVertex2f(x, y);
        }
        glEnd();
        
        // Draw outline for better visibility
        glLineWidth(1.5f);
        glColor4f(0.3f, 0.3f, 0.3f, 0.9f);  // Dark outline
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < segments; j++) {
            float angle = 2.0f * M_PI * j / segments;
            float x = map_x + cos(angle) * circle_size;
            float y = map_y + sin(angle) * circle_size;
            glVertex2f(x, y);
        }
        glEnd();
    }
    
    // Draw player position as a white dot with direction indicator
    float player_map_x = center_x;
    float player_map_y = center_y;
    
    // White color for player position
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Draw player position dot
    float player_dot_size = 6.0f;
    glPointSize(player_dot_size);
    glBegin(GL_POINTS);
    glVertex2f(player_map_x, player_map_y);
    glEnd();
    
    // Draw direction indicator line
    float direction_line_length = 15.0f;
    float dx = sinf(player->yaw * M_PI / 180.0f) * direction_line_length;
    float dz = cosf(player->yaw * M_PI / 180.0f) * direction_line_length;
    
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(player_map_x, player_map_y);
    glVertex2f(player_map_x + dx, player_map_y - dz); // Note: Z is flipped for Y in 2D
    glEnd();
    glLineWidth(1.0f);
    
    // Draw static elements as white dots with proper size
    if (static_elements != NULL) {
        // All elements are white
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Draw each active static element as a dot
        for (int i = 0; i < static_element_count; i++) {
            if (static_elements[i].active) {
                // Calculate map position relative to player
                float map_x = center_x + (static_elements[i].x - player->position_x) * map_scale;
                float map_y = center_y + (static_elements[i].z - player->position_z) * map_scale;
                
                // Draw element dot with proper size
                glPointSize(MAP_STATIC_ELEMENT_DOT_SIZE * 6.0f); // Properly scaled
                glBegin(GL_POINTS);
                glVertex2f(map_x, map_y);
                glEnd();
            }
        }
    }
    
    // Draw animals as white dots
    if (animals != NULL) {
        // All animals are white
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Draw each active animal as a dot
        for (int i = 0; i < animal_count; i++) {
            if (animals[i].active) {
                // Calculate map position relative to player
                float map_x = center_x + (animals[i].x - player->position_x) * map_scale;
                float map_y = center_y + (animals[i].z - player->position_z) * map_scale;
                
                // Draw animal dot
                glPointSize(MAP_ANIMAL_DOT_SIZE * 5.0f);
                glBegin(GL_POINTS);
                glVertex2f(map_x, map_y);
                glEnd();
            }
        }
    }
    
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