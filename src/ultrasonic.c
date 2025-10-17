#include <avr/io.h>
#include <avr/interrupt.h>
#include "../include/theremin.h"

extern system_state_t sys;

// For timing: use Timer1 with prescaler 8 -> tick = 0.5 us

void ultrasonic_init(void) {
    // set PB1 (trigger) as output, PB0 input (ICP)
    US_TRIG_DDR |= (1 << US_TRIG_PIN);
    // ensure low
    US_TRIG_PORT &= ~(1 << US_TRIG_PIN);

    // configure Timer1
    TCCR1A = 0; // normal mode
    // TCCR1B: noise canceler off; ICES1 = 1 to capture rising edge initially
    TCCR1B = (1 << ICES1) | (1 << CS11); // prescaler = 8
    // enable input capture interrupt and overflow interrupt
    TIMSK1 = (1 << ICIE1) | (1 << TOIE1);

    // clear flags
    TIFR1 = (1 << ICF1) | (1 << TOV1);
}

void ultrasonic_start_measurement(void) {
    // Start state machine: set trigger high, remember TCNT1 to time 10us
    // set us_busy flag
    sys.us_busy = true;
    sys.us_timeout = false;

    // set trigger high
    US_TRIG_PORT |= (1 << US_TRIG_PIN);

    // We'll clear trigger after ~10us by waiting until TCNT1 advanced by >=20 ticks (0.5us * 20 = 10us)
    // Note: non-blocking approach in main checks for sys.us_busy and relies on timer value
    // To implement clearing after 10us without blocking here, we spin briefly but extremely small to be safe:
    // Instead perform a short busy-wait using TCNT1 (fast).
    uint16_t start = TCNT1;
    while ((uint16_t)(TCNT1 - start) < 20) {
        // tiny busy wait; this is only ~10 microseconds because Timer1 ticks at 0.5us.
        // This is acceptable because it's very short and necessary for the trigger pulse.
        // No Arduino delay used as requested.
    }
    // set trigger low
    US_TRIG_PORT &= ~(1 << US_TRIG_PIN);

    // Prepare Timer1 capture: ensure ICES1 = 1 (rising edge)
    TCCR1B |= (1 << ICES1);

    // reset counter to 0 to start measuring after rising
    TCNT1 = 0;

    // ensure flags clear
    TIFR1 = (1 << ICF1);

    // The Input Capture ISR will set sys.new_distance_ready when both edges detected.
}

// Input Capture interrupt: catch rising and falling edges
ISR(TIMER1_CAPT_vect) {
    static uint8_t edge = 0; // 0 = waiting for rising, 1 = waiting for falling
    if ( (TCCR1B & (1 << ICES1)) ) {
        // captured rising edge (shouldn't be used for width)
        // reset timer counter and switch to falling edge
        TCNT1 = 0;
        // set ICES1 = 0 for falling
        TCCR1B &= ~(1 << ICES1);
    } else {
        // falling edge captured, ICR1 holds pulse width in ticks
        uint16_t v = ICR1;
        sys.us_capture_value = v;
        sys.new_distance_ready = true;
        sys.us_busy = false;
        // switch back to rising
        TCCR1B |= (1 << ICES1);
    }
}

// overflow: if no echo returns in time (timeout)
ISR(TIMER1_OVF_vect) {
    sys.us_timeout = true;
    sys.us_busy = false;
    // clear overflow flag handled by hardware
}
