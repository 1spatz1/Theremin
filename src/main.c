#include <avr/io.h>
#include <avr/interrupt.h>
#include "theremin.h"
#include "adc.h"
#include "ultrasonic.h"
#include "filter.h"
#include "buttons.h"
#include "buzzer.h"
#include "display.h"
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
    buttons_init();
    buzzer_init();
    display_init();

    sei();

    // uart for debug
    // uart_init(9600);
    // uart_println("Theremin debug start");

    uint16_t raw_distance_cm = 0;
    uint16_t filtered_distance_cm = 0;
    uint16_t frequency_hz = FREQ_MIN;

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

            // map filtered distance to frequency: 0..65cm -> 1400..230
            uint32_t delta = (uint32_t)(FREQ_MAX - FREQ_MIN); // 1170
            uint32_t freq = FREQ_MAX;
            // linear mapping: freq = FREQ_MAX - (dist * delta)/MAX_DISTANCE_CM
            freq = FREQ_MAX - ((uint32_t)filtered_distance_cm * delta) / MAX_DISTANCE_CM;
            if (freq < FREQ_MIN) freq = FREQ_MIN;
            if (freq > FREQ_MAX) freq = FREQ_MAX;
            frequency_hz = (uint16_t)freq;

            // update buzzer frequency
            buzzer_set_frequency(frequency_hz);

            display_update(filtered_distance_cm, freq, sys.filter_size);
        }

        // uart for debug
        // if (sys.new_distance_ready) {
        //     uart_print("Dist: ");
        //     uart_print_uint16(filtered_distance_cm);
        //     uart_print(" cm | Freq: ");
        //     uart_print_uint16(frequency_hz);
        //     uart_print(" Hz | Filter: ");
        //     uart_print_uint8(sys.filter_size);
        //     uart_println("");
        // }

    }
    return 0;
}
