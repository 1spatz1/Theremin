#include "../include/mapping.h"
#include "../include/theremin.h"

// Convert Timer1 ticks (prescaler=8, tick=0.5us) to distance in cm
// From main.c: distance_cm = ticks / 116
uint16_t ticks_to_distance_cm(uint16_t ticks) {
    uint16_t dist = ticks / 116;
    if (dist > MAX_DISTANCE_CM) dist = MAX_DISTANCE_CM;
    return dist;
}

// Map distance (0..MAX_DISTANCE_CM) to frequency (FREQ_MAX..FREQ_MIN)
uint16_t distance_to_frequency(uint16_t dist_cm) {
    if (dist_cm > MAX_DISTANCE_CM) dist_cm = MAX_DISTANCE_CM;
    uint32_t delta = (uint32_t)(FREQ_MAX - FREQ_MIN);
    uint32_t freq = FREQ_MAX;
    freq = FREQ_MAX - ((uint32_t)dist_cm * delta) / MAX_DISTANCE_CM;
    if (freq < FREQ_MIN) freq = FREQ_MIN;
    if (freq > FREQ_MAX) freq = FREQ_MAX;
    return (uint16_t)freq;
}
