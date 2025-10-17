#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void display_init(void);
void display_update(uint16_t distance_cm, uint16_t freq_hz, uint8_t filter_value);

#endif
