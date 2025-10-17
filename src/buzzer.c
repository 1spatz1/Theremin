#include <avr/io.h>
#include <avr/interrupt.h>
#include "../include/theremin.h"
#include "../include/buzzer.h"

extern system_state_t sys;

// Timer0 used for frequency CTC to create half-period interrupts
// Timer2 used for fast PWM that produces the carrier; OCR2B duty set by ADC

void buzzer_init(void) {
    // set PD3 (OC2B) as output
    BUZZER_DDR |= (1 << BUZZER_PIN);

    // Timer2: Fast PWM (WGM21:0 = 3), non-inverting on OC2B (COM2B1 = 1)
    // We'll enable/disable COM2B1 in Timer0 ISR to gate the PWM
    TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2B1); // start with enabled
    TCCR2B = (1 << CS20); // no prescaling

    // initial duty (OCR2B) set by ADC ISR; default 0
    OCR2B = 0;

    // Timer0: CTC mode (WGM01 = 1), prescaler = 256 (CS02 = 1)
    TCCR0A = (1 << WGM01); // CTC
    TCCR0B = (1 << CS02); // prescale 256

    // enable compare match A interrupt
    TIMSK0 = (1 << OCIE0A);

    // initial OCR0A set for minimum frequency
    buzzer_set_frequency(FREQ_MIN);
}

// compute OCR0A for desired frequency: OCR0A = F_CPU / (prescaler * 2 * freq) - 1
static uint8_t compute_ocr0a_for_freq(uint16_t freq) {
    uint32_t prescaler = 256UL;
    uint32_t val = (F_CPU / (prescaler * 2UL * (uint32_t)freq));
    if (val == 0) return 0;
    if (val > 255) return 255;
    return (uint8_t)(val - 1);
}

void buzzer_set_frequency(uint16_t freq_hz) {
    if (freq_hz < FREQ_MIN) freq_hz = FREQ_MIN;
    if (freq_hz > FREQ_MAX) freq_hz = FREQ_MAX;
    uint8_t ocr = compute_ocr0a_for_freq(freq_hz);
    // update OCR0A atomically
    uint8_t sreg = SREG;
    cli();
    OCR0A = ocr;
    SREG = sreg;
}

// Timer0 Compare Match A ISR: toggles gating of OC2B (enable/disable PWM output)
// We'll toggle COM2B1 bit in TCCR2A to simulate enabling/disabling the PWM output
ISR(TIMER0_COMPA_vect) {
    static uint8_t pulse = 0;
    pulse ^= 1;
    if (pulse) {
        // enable OC2B (non-inverting) => set COM2B1
        TCCR2A |= (1 << COM2B1);
    } else {
        // disable output by clearing COM2B1
        TCCR2A &= ~(1 << COM2B1);
    }
}
