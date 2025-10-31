#ifndef BUZZER_H
#define BUZZER_H

void buzzer_init(void);
void buzzer_set_frequency(uint16_t freq_hz);
uint8_t compute_ocr0a_for_freq(uint16_t freq);

#endif
