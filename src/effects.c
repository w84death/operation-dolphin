#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "../include/effects.h"

// Static variable to store the shake offsets that will be returned
static float shake_offsets[2] = {0.0f, 0.0f};

// Initialize shake system
void initShake(void) {
    shake_offsets[0] = 0.0f; // X offset
    shake_offsets[1] = 0.0f; // Y offset
}

// Calculate screen shake based on current speed
// Returns a pointer to a 2-element array containing [x_offset, y_offset]
float* calculateShake(float current_speed, float max_speed, float max_shake_amount, bool game_started) {
    // Reset shake offsets
    shake_offsets[0] = 0.0f;
    shake_offsets[1] = 0.0f;
    
    if (game_started && current_speed > 0) {
        // Intensity based on speed (0 to 1)
        float speed_ratio = fmin(current_speed / max_speed, 1.0f);
        
        // Apply a curve to make shake more pronounced at higher speeds
        float shake_intensity = pow(speed_ratio, 2.0f) * max_shake_amount;
        
        // Generate random offsets within the intensity range
        shake_offsets[0] = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * shake_intensity; // X offset
        shake_offsets[1] = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * shake_intensity; // Y offset
    }
    
    return shake_offsets;
}