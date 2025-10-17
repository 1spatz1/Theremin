#include <avr/io.h>
#include <avr/interrupt.h>
#include "theremin.h"
#include "adc.h"
#include "ultrasonic.h"
#include "filter.h"
#include "uart.h"

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
    filter_init();

    sei();

    uart_init(9600);
    uart_println("Theremin debug start");

    uint16_t raw_distance_cm = 0;
    uint16_t filtered_distance_cm = 0;

    // main non-blocking loop
    while (1) {
        // Trigger ultrasonic measurement whenever not busy
        if (!sys.us_busy) {
            ultrasonic_start_measurement();
        }

        // If a new capture result is available, process it
        if (sys.new_distance_ready) {
            cli();
            uint16_t ticks = sys.us_capture_value;
            sys.new_distance_ready = false;
            sei();

            // compute distance in cm
            // Timer1 prescaler = 8 -> tick = 0.5 us
            // time_us = ticks * 0.5
            // distance_cm = time_us / 58 = (ticks / 2) / 58 = ticks / 116
            uint16_t dist_cm = ticks / 116;
            if (dist_cm > MAX_DISTANCE_CM) dist_cm = MAX_DISTANCE_CM;
            raw_distance_cm = dist_cm;

            // add to filter and get filtered distance
            filtered_distance_cm = filter_add_sample_and_get_median(raw_distance_cm);

        }
        if (sys.new_distance_ready) {
            // ... compute filtered_distance_cm and freq
            uart_print("Dist: ");
            uart_print_uint16(filtered_distance_cm);
            uart_print("Filter: ");
            uart_print_uint8(sys.filter_size);
            uart_println("");
        }

    }
    return 0;
}
