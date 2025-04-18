#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdbool.h>

// Function declarations for screen shake effects
void initShake(void);
float* calculateShake(float current_speed, float max_speed, float max_shake_amount, bool game_started);

#endif // EFFECTS_H