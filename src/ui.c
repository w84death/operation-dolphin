#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ui.h"
#include "../include/log.h"

// Initialize UI system
bool initUI(UISystem* ui, const char* font_path, int font_size, int screen_width, int screen_height) {
    // Initialize SDL_ttf if not already initialized
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            log_error("SDL_ttf could not initialize! SDL_ttf Error: %s", TTF_GetError());
            return false;
        }
    }
    
    // Load font
    ui->font = TTF_OpenFont(font_path, font_size);
    if (ui->font == NULL) {
        log_error("Failed to load font! SDL_ttf Error: %s", TTF_GetError());
        return false;
    }
    
    // Allocate memory for UI elements
    ui->max_elements = 20; // Start with space for 20 elements
    ui->elements = (UIElement*)malloc(ui->max_elements * sizeof(UIElement));
    if (ui->elements == NULL) {
        log_error("Failed to allocate memory for UI elements");
        TTF_CloseFont(ui->font);
        return false;
    }
    
    ui->element_count = 0;
    ui->screen_width = screen_width;
    ui->screen_height = screen_height;
    
    log_success("UI system initialized successfully with font: %s", font_path);
    return true;
}

// Clean up UI system resources
void cleanupUI(UISystem* ui) {
    if (ui == NULL) return;
    
    // Free each text texture and string
    for (int i = 0; i < ui->element_count; i++) {
        if (ui->elements[i].text != NULL) {
            free(ui->elements[i].text);
        }
        if (ui->elements[i].texture_id != 0) {
            glDeleteTextures(1, &ui->elements[i].texture_id);
        }
    }
    
    // Free element array
    if (ui->elements != NULL) {
        free(ui->elements);
        ui->elements = NULL;
    }
    
    // Close font
    if (ui->font != NULL) {
        TTF_CloseFont(ui->font);
        ui->font = NULL;
    }
    
    // Quit SDL_ttf
    TTF_Quit();
}

// Create a text element and return its ID
int createTextElement(UISystem* ui, const char* text, int x, int y, SDL_Color color, TextAlignment alignment) {
    // Check if we need to expand the elements array
    if (ui->element_count >= ui->max_elements) {
        int new_max = ui->max_elements * 2;
        UIElement* new_elements = (UIElement*)realloc(ui->elements, new_max * sizeof(UIElement));
        if (new_elements == NULL) {
            log_error("Failed to reallocate memory for UI elements");
            return -1;
        }
        ui->elements = new_elements;
        ui->max_elements = new_max;
    }
    
    // Initialize the new element
    int id = ui->element_count;
    ui->elements[id].type = UI_TEXT;
    ui->elements[id].x = x;
    ui->elements[id].y = y;
    ui->elements[id].visible = true;
    ui->elements[id].color = color;
    ui->elements[id].alignment = alignment;
    ui->elements[id].texture_id = 0;
    
    // Copy the text
    ui->elements[id].text = strdup(text);
    if (ui->elements[id].text == NULL) {
        log_error("Failed to allocate memory for text");
        return -1;
    }
    
    // Render the text to a texture
    SDL_Surface* text_surface = TTF_RenderText_Blended(ui->font, text, color);
    if (text_surface == NULL) {
        log_error("Unable to render text surface! SDL_ttf Error: %s", TTF_GetError());
        free(ui->elements[id].text);
        return -1;
    }
    
    // Convert the surface to the correct format for OpenGL
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    
    SDL_Surface* gl_surface = SDL_CreateRGBSurface(0, text_surface->w, text_surface->h, 32, 
                                                  rmask, gmask, bmask, amask);
    if (gl_surface == NULL) {
        log_error("Unable to create RGB surface! SDL Error: %s", SDL_GetError());
        SDL_FreeSurface(text_surface);
        free(ui->elements[id].text);
        return -1;
    }
    
    // Blend the text onto the RGB surface
    SDL_BlitSurface(text_surface, NULL, gl_surface, NULL);
    SDL_FreeSurface(text_surface);
    
    // Create OpenGL texture from surface
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_surface->w, gl_surface->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, gl_surface->pixels);
                
    // Store texture ID and dimensions
    ui->elements[id].texture_id = texture_id;
    ui->elements[id].width = gl_surface->w;
    ui->elements[id].height = gl_surface->h;
    
    // Free surface
    SDL_FreeSurface(gl_surface);
    
    // Increment element count
    ui->element_count++;
    
    log_success("Created text element '%s' with ID %d, size: %dx%d", 
           text, id, ui->elements[id].width, ui->elements[id].height);
    
    return id;
}

// Update the text of an existing text element
void updateTextElement(UISystem* ui, int element_id, const char* text) {
    if (element_id < 0 || element_id >= ui->element_count) {
        log_error("Invalid element ID: %d", element_id);
        return;
    }
    
    UIElement* element = &ui->elements[element_id];
    
    // Skip if text hasn't changed
    if (strcmp(element->text, text) == 0) {
        return;
    }
    
    // Delete old text
    if (element->text != NULL) {
        free(element->text);
    }
    
    // Copy new text
    element->text = strdup(text);
    if (element->text == NULL) {
        log_error("Failed to allocate memory for text");
        return;
    }
    
    // Delete old texture
    if (element->texture_id != 0) {
        glDeleteTextures(1, &element->texture_id);
    }
    
    // Render the text to a texture
    SDL_Surface* text_surface = TTF_RenderText_Blended(ui->font, text, element->color);
    if (text_surface == NULL) {
        log_error("Unable to render text surface! SDL_ttf Error: %s", TTF_GetError());
        return;
    }
    
    // Convert the surface to the correct format for OpenGL
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    
    SDL_Surface* gl_surface = SDL_CreateRGBSurface(0, text_surface->w, text_surface->h, 32, 
                                                  rmask, gmask, bmask, amask);
    if (gl_surface == NULL) {
        log_error("Unable to create RGB surface! SDL Error: %s", SDL_GetError());
        SDL_FreeSurface(text_surface);
        return;
    }
    
    // Blend the text onto the RGB surface
    SDL_BlitSurface(text_surface, NULL, gl_surface, NULL);
    SDL_FreeSurface(text_surface);
    
    // Create OpenGL texture from surface
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_surface->w, gl_surface->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, gl_surface->pixels);
                
    // Store texture ID and dimensions
    element->texture_id = texture_id;
    element->width = gl_surface->w;
    element->height = gl_surface->h;
    
    // Free surface
    SDL_FreeSurface(gl_surface);
}

// Update the color of an existing text element
void updateTextColor(UISystem* ui, int element_id) {
    if (element_id < 0 || element_id >= ui->element_count) {
        log_error("Invalid element ID: %d", element_id);
        return;
    }
    
    UIElement* element = &ui->elements[element_id];
    
    // Delete old texture if it exists
    if (element->texture_id != 0) {
        glDeleteTextures(1, &element->texture_id);
        element->texture_id = 0;
    }
    
    // Create a new SDL text surface with the current color
    SDL_Surface* text_surface = TTF_RenderText_Blended(ui->font, element->text, element->color);
    if (text_surface == NULL) {
        log_error("Failed to render text with new color! SDL_ttf Error: %s", TTF_GetError());
        return;
    }
    
    // Create RGB surface for OpenGL
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    
    SDL_Surface* gl_surface = SDL_CreateRGBSurface(0, text_surface->w, text_surface->h, 32,
                                                 rmask, gmask, bmask, amask);
    if (gl_surface == NULL) {
        log_error("Failed to create RGB surface! SDL Error: %s", SDL_GetError());
        SDL_FreeSurface(text_surface);
        return;
    }
    
    // Blend text onto RGB surface
    SDL_BlitSurface(text_surface, NULL, gl_surface, NULL);
    SDL_FreeSurface(text_surface);
    
    // Create new OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_surface->w, gl_surface->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, gl_surface->pixels);
    
    // Store the new texture ID and dimensions
    element->texture_id = texture_id;
    element->width = gl_surface->w;
    element->height = gl_surface->h;
    
    // Free the surface
    SDL_FreeSurface(gl_surface);
}

// Set element visibility
void setElementVisibility(UISystem* ui, int element_id, bool visible) {
    if (element_id < 0 || element_id >= ui->element_count) {
        log_error("Invalid element ID: %d", element_id);
        return;
    }
    
    ui->elements[element_id].visible = visible;
}

// Set element position
void setElementPosition(UISystem* ui, int element_id, int x, int y) {
    if (element_id < 0 || element_id >= ui->element_count) {
        log_error("Invalid element ID: %d", element_id);
        return;
    }
    
    ui->elements[element_id].x = x;
    ui->elements[element_id].y = y;
}

// Set element color
void setElementColor(UISystem* ui, int element_id, SDL_Color color) {
    if (element_id < 0 || element_id >= ui->element_count) {
        log_error("Invalid element ID: %d", element_id);
        return;
    }
    
    UIElement* element = &ui->elements[element_id];
    
    // Update the color
    element->color = color;
    
    // Use our specialized function to update the texture with the new color
    updateTextColor(ui, element_id);
}

// Reposition all UI elements when screen resolution changes
void repositionUI(UISystem* ui, int new_width, int new_height) {
    if (ui == NULL || ui->screen_width == 0 || ui->screen_height == 0) {
        return;
    }
    
    // Calculate scale factors between old and new resolution
    float scale_x = (float)new_width / (float)ui->screen_width;
    float scale_y = (float)new_height / (float)ui->screen_height;
    
    // Update each UI element's position
    for (int i = 0; i < ui->element_count; i++) {
        UIElement* element = &ui->elements[i];
        
        // Scale position based on new resolution
        int new_x, new_y;
        
        // Handle alignment specially
        if (element->alignment == TEXT_ALIGN_LEFT) {
            // Left-aligned elements: maintain same margin from left
            new_x = (int)(element->x * scale_x);
        } 
        else if (element->alignment == TEXT_ALIGN_RIGHT) {
            // Right-aligned elements: maintain same margin from right
            new_x = new_width - (int)((ui->screen_width - element->x) * scale_x);
        }
        else { // TEXT_ALIGN_CENTER
            // Center-aligned elements: maintain proportional position
            new_x = (int)(element->x * scale_x);
        }
        
        // Y position is scaled proportionally for all elements
        new_y = (int)(element->y * scale_y);
        
        // Update element position
        element->x = new_x;
        element->y = new_y;
    }
    
    // Update screen dimensions in the UI system
    ui->screen_width = new_width;
    ui->screen_height = new_height;
    
    log_success("UI repositioned for new resolution: %dx%d", new_width, new_height);
}

// Render all visible UI elements
void renderUI(UISystem* ui) {
    // Save current matrices and attributes
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, ui->screen_width, ui->screen_height, 0.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Save GL states
    GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lighting_enabled = glIsEnabled(GL_LIGHTING);
    GLboolean fog_enabled = glIsEnabled(GL_FOG);
    GLboolean cull_face_enabled = glIsEnabled(GL_CULL_FACE);
    GLint blend_src, blend_dst;
    glGetIntegerv(GL_BLEND_SRC, &blend_src);
    glGetIntegerv(GL_BLEND_DST, &blend_dst);
    
    // Disable 3D features for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    
    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    
    // Render each visible UI element
    for (int i = 0; i < ui->element_count; i++) {
        UIElement* element = &ui->elements[i];
        
        if (!element->visible || element->texture_id == 0) {
            continue;
        }
        
        // Calculate position based on alignment
        int x = element->x;
        if (element->alignment == TEXT_ALIGN_CENTER) {
            x -= element->width / 2;
        } else if (element->alignment == TEXT_ALIGN_RIGHT) {
            x -= element->width;
        }
        
        // Bind texture
        glBindTexture(GL_TEXTURE_2D, element->texture_id);
        
        // Set color to white to show texture's colors as they are
        // (textures already have the correct color from rendering)
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Draw quad with texture
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, element->y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + element->width, element->y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + element->width, element->y + element->height);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, element->y + element->height);
        glEnd();
    }
    
    // Reset texture binding
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Restore GL states
    if (depth_test_enabled) glEnable(GL_DEPTH_TEST);
    if (lighting_enabled) glEnable(GL_LIGHTING);
    if (fog_enabled) glEnable(GL_FOG);
    if (cull_face_enabled) glEnable(GL_CULL_FACE);
    glBlendFunc(blend_src, blend_dst);
    
    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}