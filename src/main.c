#include <avr/io.h>
#include <avr/interrupt.h>
#include "theremin.h"
#include "adc.h"

system_state_t sys;

int main(void) {
    cli();

    // default state
    sys.adc_volume = 0;
    sys.filter_size = 5; // default odd between 1..15

    // init modules
    adc_init();

    sei();
    // minimal skeleton: infinite loop
    while (1) {

    }
    return 0;
}
