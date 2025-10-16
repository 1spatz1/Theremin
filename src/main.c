#include <avr/io.h>
#include <avr/interrupt.h>
#include "theremin.h"
#include "adc.h"
#include "ultrasonic.h"

system_state_t sys;

int main(void) {
    cli();

    // default state
    sys.adc_volume = 0;
    sys.last_pulse_ticks = 0;
    sys.us_busy = false;
    sys.new_distance_ready = false;
    sys.us_timeout = false;
    sys.us_capture_value = 0;
    sys.filter_size = 5; // default odd between 1..15

    // init modules
    adc_init();
    ultrasonic_init();

    sei();
    // minimal skeleton: infinite loop
    while (1) {
        // Trigger ultrasonic measurement whenever not busy
        if (!sys.us_busy) {
            ultrasonic_start_measurement();
        }

    }
    return 0;
}
