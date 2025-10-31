// mapping.h - helpers for converting timer ticks and distances to frequency
#ifndef MAPPING_H
#define MAPPING_H

#include <stdint.h>

uint16_t ticks_to_distance_cm(uint16_t ticks);
uint16_t distance_to_frequency(uint16_t dist_cm);

#endif // MAPPING_H
