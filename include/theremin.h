#ifndef THEREMIN_H
#define THEREMIN_H

#include <stdint.h>
#include <stdbool.h>

// hardware pins (Arduino UNO mapping)
#define BUZZER_PORT PORTD
#define BUZZER_DDR  DDRD
#define BUZZER_PIN  PD3   // OC2B

#define US_TRIG_PORT PORTB
#define US_TRIG_DDR  DDRB
#define US_TRIG_PIN  PB1   // trigger output

#define US_ECHO_PIN  PB0   // echo input (ICP1)

#define BTN_PORT_PIN PIND
#define BTN_PORT     PORTD
#define BTN_PIN0     PD4
#define BTN_PIN1     PD5

// filter limits
#define FILTER_MIN 1
#define FILTER_MAX 15

// frequency limits
#define FREQ_MIN 230
#define FREQ_MAX 1400

// ultrasonic range
#define MAX_DISTANCE_CM 65

// shared state
typedef struct {
    volatile uint8_t adc_volume; // 0..255
    volatile uint16_t last_pulse_ticks; // timer1 ticks measured (ICR1 units)
    volatile bool new_distance_ready;
    volatile bool us_timeout;
    volatile bool us_busy;
    volatile uint16_t us_capture_value;
    volatile uint8_t filter_size; // odd number 1..15
} system_state_t;

extern system_state_t sys;

#endif // THEREMIN_H
