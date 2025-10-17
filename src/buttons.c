#include <avr/io.h>
#include <avr/interrupt.h>
#include "theremin.h"
#include "../include/filter.h"

extern system_state_t sys;

void buttons_init(void) {
    // PD4 and PD5 as inputs with internal pull-ups
    DDRD &= ~((1 << PD4) | (1 << PD5));
    PORTD |= (1 << PD4) | (1 << PD5);

    // enable Pin Change Interrupt for PCINT2 (PCINT16..23)
    PCICR |= (1 << PCIE2);

    // enable PCINT20 (PD4) and PCINT21 (PD5)
    PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);
}

// simple edge detection with latch
ISR(PCINT2_vect) {
    static uint8_t last = 0xFF;
    uint8_t cur = PIND;
    uint8_t changed = last ^ cur;
    last = cur;

    // buttons are active-low due to pull-up
    if (changed & (1 << PD4)) {
        if (!(cur & (1 << PD4))) {
            // PD4 pressed -> decrease filter by 2
            uint8_t s = filter_get_size();
            if (s > FILTER_MIN) {
                if (s <= 2) s = 1;
                else s -= 2;
                filter_set_size(s);
            }
        }
    }
    if (changed & (1 << PD5)) {
        if (!(cur & (1 << PD5))) {
            // PD5 pressed -> increase filter by 2
            uint8_t s = filter_get_size();
            if (s < FILTER_MAX) {
                s += 2;
                if (s > FILTER_MAX) s = FILTER_MAX;
                filter_set_size(s);
            }
        }
    }
}
