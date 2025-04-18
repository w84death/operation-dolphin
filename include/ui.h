#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <GL/gl.h>

// UI element types
typedef enum {
    UI_TEXT,
    UI_IMAGE
} UIElementType;

// Text alignment
typedef enum {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
} TextAlignment;

// UI element structure
typedef struct {
    UIElementType type;
    int x, y;                 // Screen position
    bool visible;             // Is the element visible
    SDL_Color color;          // Text color
    TextAlignment alignment;  // Text alignment
    char* text;               // Text content (dynamically allocated)
    GLuint texture_id;        // OpenGL texture ID for rendered text
    int width, height;        // Dimensions of the rendered text
} UIElement;

// UI system structure
typedef struct {
    TTF_Font* font;
    UIElement* elements;
    int element_count;
    int max_elements;
    int screen_width;
    int screen_height;
} UISystem;

// Function declarations
bool initUI(UISystem* ui, const char* font_path, int font_size, int screen_width, int screen_height);
void cleanupUI(UISystem* ui);

// Element creation/management
int createTextElement(UISystem* ui, const char* text, int x, int y, SDL_Color color, TextAlignment alignment);
void updateTextElement(UISystem* ui, int element_id, const char* text);
void setElementVisibility(UISystem* ui, int element_id, bool visible);
void setElementPosition(UISystem* ui, int element_id, int x, int y);
void setElementColor(UISystem* ui, int element_id, SDL_Color color);
void repositionUI(UISystem* ui, int new_width, int new_height);

// Rendering
void renderUI(UISystem* ui);

#endif // UI_H