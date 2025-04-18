#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <image_file>\n", argv[0]);
        return 1;
    }
    
    int width, height, channels;
    unsigned char* image = stbi_load(argv[1], &width, &height, &channels, 0);
    if (!image) {
        printf("Failed to load image: %s\n", argv[1]);
        printf("STB Error: %s\n", stbi_failure_reason());
        return 1;
    }
    
    printf("Successfully loaded image: %s (%dx%d with %d channels)\n", 
           argv[1], width, height, channels);
    
    stbi_image_free(image);
    return 0;
}
