#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

void filter_init(void);
uint16_t filter_add_sample_and_get_median(uint16_t sample);
void filter_set_size(uint8_t size);
uint8_t filter_get_size(void);

#endif
