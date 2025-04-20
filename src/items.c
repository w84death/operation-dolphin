#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "../include/items.h"
#include "../include/config.h"
#include "../include/log.h"
#include "../stb_image.h" // For texture loading

// Global items array
static Item items[MAX_ITEMS];
static int item_count = 0;

// Texture IDs
static GLuint box_texture_closed = 0;
static GLuint box_texture_open = 0;
static GLuint ammo_texture_closed = 0;
static GLuint ammo_texture_open = 0;

// Forward declaration for utility function
static GLuint loadItemTexture(const char* texture_path);
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture);

// Initialize the items system and load textures
bool initItems(void) {
    log_info("Initializing items system...");
    
    // Load textures for items
    box_texture_closed = loadItemTexture(ITEM_BOX_TEXTURE_CLOSED);
    box_texture_open = loadItemTexture(ITEM_BOX_TEXTURE_OPEN);
    ammo_texture_closed = loadItemTexture(ITEM_AMMO_TEXTURE_CLOSED);
    ammo_texture_open = loadItemTexture(ITEM_AMMO_TEXTURE_OPEN);
    
    // Check if texture loading was successful
    if (box_texture_closed == 0 || box_texture_open == 0 || 
        ammo_texture_closed == 0 || ammo_texture_open == 0) {
        log_error("Failed to load item textures");
        return false;
    }
    
    // Initialize all items as inactive
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
        items[i].opened = false;
    }
    
    log_success("Items system initialized successfully");
    return true;
}

// Create items randomly distributed on the terrain
void createItems(int count, float terrain_size, Terrain* terrain) {
    log_info("Creating %d items...", count);
    
    // Reset item count
    item_count = 0;
    
    // Clear any existing items
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
        items[i].opened = false;
    }
    
    // Limit count to maximum
    if (count > MAX_ITEMS) {
        count = MAX_ITEMS;
        log_warning("Item count limited to %d", MAX_ITEMS);
    }
    
    // Create random items
    for (int i = 0; i < count; i++) {
        // Generate random position within terrain bounds
        float x = ((float)rand() / RAND_MAX - 0.5f) * terrain_size;
        float z = ((float)rand() / RAND_MAX - 0.5f) * terrain_size;
        
        // Get the terrain height at this position
        float y = getHeightAtPoint(terrain, x, z);
        
        // Add a small offset to ensure items sit on top of the terrain and don't clip through
        y += ITEM_BOX_HEIGHT * 0.5f;
        
        // Randomly pick item type
        ItemType type = (rand() % 2 == 0) ? ITEM_TYPE_BOX : ITEM_TYPE_AMMO;
        
        // Set up item
        items[i].x = x;
        items[i].y = y;
        items[i].z = z;
        items[i].width = ITEM_BOX_WIDTH;
        items[i].height = ITEM_BOX_HEIGHT;
        items[i].active = true;
        items[i].opened = false;
        items[i].type = type;
        
        // Assign textures based on type
        if (type == ITEM_TYPE_BOX) {
            items[i].texture_closed = box_texture_closed;
            items[i].texture_open = box_texture_open;
        } else {
            items[i].texture_closed = ammo_texture_closed;
            items[i].texture_open = ammo_texture_open;
        }
        
        item_count++;
    }
    
    log_success("Created %d items", item_count);
}

// Render all active items
void renderItems(void) {
    // Save current material properties and states
    float orig_ambient[4], orig_diffuse[4];
    glGetMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glGetMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
    
    // Material for items - brighter to make them stand out more
    float ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    // Set material to fully accept light without changing the color
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    
    // Make sure textures properly interact with lighting
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Save depth test state and temporarily disable depth writing for alpha blending
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glDepthMask(GL_FALSE);
    
    // Render all items
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i].active) {
            // Choose texture based on opened state
            GLuint texture = items[i].opened ? items[i].texture_open : items[i].texture_closed;
            
            // Draw the item as a billboard
            drawBillboard(
                items[i].x, 
                items[i].y, 
                items[i].z, 
                items[i].width, 
                items[i].height, 
                texture
            );
        }
    }
    
    // Restore depth writing state
    glDepthMask(depthMask);
    
    // Restore original material state
    glMaterialfv(GL_FRONT, GL_AMBIENT, orig_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orig_diffuse);
}

// Check for collisions between the player and items
void checkItemCollisions(Player* player) {
    // Only check for collisions if player is actively in the game
    if (player == NULL) return;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        // Skip inactive or already opened items
        if (!items[i].active || items[i].opened) continue;
        
        // Calculate distance to player
        float dx = items[i].x - player->position_x;
        float dz = items[i].z - player->position_z;
        float distance = sqrtf(dx * dx + dz * dz);
        
        // Check if within interaction range
        if (distance < ITEM_INTERACTION_RANGE) {
            // Open the item
            items[i].opened = true;
            
            // Log the interaction
            const char* type_str = (items[i].type == ITEM_TYPE_BOX) ? "crate box" : "ammo box";
            log_info("Player opened %s at position [%.2f, %.2f, %.2f]", type_str, 
                    items[i].x, items[i].y, items[i].z);
            
            // In future versions, this would give the player the item contents
            // For now, we're just changing the texture
        }
    }
}

// Clean up item resources
void cleanupItems(void) {
    log_info("Cleaning up items resources...");
    
    // Delete textures
    if (box_texture_closed != 0) glDeleteTextures(1, &box_texture_closed);
    if (box_texture_open != 0) glDeleteTextures(1, &box_texture_open);
    if (ammo_texture_closed != 0) glDeleteTextures(1, &ammo_texture_closed);
    if (ammo_texture_open != 0) glDeleteTextures(1, &ammo_texture_open);
    
    // Reset all items
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
    }
    
    item_count = 0;
}

// Load a texture from file using stb_image
static GLuint loadItemTexture(const char* texture_path) {
    log_info("Loading item texture: %s", texture_path);
    
    // Load image using stb_image
    int width, height, channels;
    unsigned char *image = stbi_load(texture_path, &width, &height, &channels, STBI_rgb_alpha);
    
    if (!image) {
        log_error("Failed to load texture: %s", stbi_failure_reason());
        return 0;
    }
    
    // Generate OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // Free the image data as it's now in GPU memory
    stbi_image_free(image);
    
    log_success("Texture loaded successfully, ID: %u", texture_id);
    return texture_id;
}

// Draw a billboard (quad that always faces the camera)
static void drawBillboard(float x, float y, float z, float width, float height, GLuint texture) {
    // Only draw if we have a valid texture
    if (texture == 0) return;
    
    // Get the current modelview matrix
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    // Extract the right and up vectors from the modelview matrix
    float right[3] = {modelview[0], modelview[4], modelview[8]};
    float up[3] = {modelview[1], modelview[5], modelview[9]};
    
    // Calculate the billboard corners
    float half_width = width * 0.5f;
    float half_height = height * 0.5f;
        
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_TEXTURE_2D);
    
    // Enable alpha blending for transparent textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth test to ensure the item is always visible
    glDisable(GL_DEPTH_TEST);
    
    // Draw the billboard quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - right[0] * half_width + up[0] * half_height,
               y - right[1] * half_width + up[1] * half_height,
               z - right[2] * half_width + up[2] * half_height);
    
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + right[0] * half_width + up[0] * half_height,
               y + right[1] * half_width + up[1] * half_height,
               z + right[2] * half_width + up[2] * half_height);
    
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + right[0] * half_width - up[0] * half_height,
               y + right[1] * half_width - up[1] * half_height,
               z + right[2] * half_width - up[2] * half_height);
    
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x - right[0] * half_width - up[0] * half_height,
               y - right[1] * half_width - up[1] * half_height,
               z - right[2] * half_width - up[2] * half_height);
    glEnd();
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);
}