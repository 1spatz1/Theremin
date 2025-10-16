#include <avr/io.h>
#include <avr/interrupt.h>
#include "theremin.h"

extern system_state_t sys;

void adc_init(void) {
    // ADMUX: REFS0 = 1 (AVcc), ADLAR = 1 (left adjust), MUX = 0 (ADC0)
    ADMUX = (1 << REFS0) | (1 << ADLAR);

    // ADCSRA: ADEN, ADSC, ADATE, ADIE, prescaler 128 (111)
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // ADCSRB: Free running (ADTS = 000)
    ADCSRB = 0;
}

// ADC complete interrupt
ISR(ADC_vect) {
    // read high 8 bits from ADCH (because ADLAR=1)
    uint8_t v = ADCH;
    sys.adc_volume = v;

    // map volume to PWM duty (OCR2B) -- scale 0..255 to 0..255 directly
    OCR2B = v; // buzzer module will use OCR2B
}
