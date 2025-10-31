#include "unity.h"
#include "../include/theremin.h"
#include <stdint.h>

/* provide shared state used by filter.c */
system_state_t sys;

/* include unit-under-test directly for host testing */
#include "../src/filter.c"

/* --- UART unit tests (included here to provide a single Unity runner) --- */
/* include uart implementation */
#include "../src/uart.c"

/* display / TWI stubs moved to shared test helpers */
#include "support/test_helpers.h"
/* include the implementation so helper symbols are available in the test TU */
#include "support/test_helpers.c"



/* include small modules under test */
#include "../src/adc.c"
#include "../src/buzzer.c"
#include "../src/buttons.c"
#include "../src/display.c"
#include "../src/mapping.c"
/* include ultrasonic implementation to test ISRs (we won't call blocking start) */
#include "../src/ultrasonic.c"

void setUp(void) { }
void tearDown(void) { }

void test_filter_init_sets_defaults(void) {
    /* Ensure filter_init sets default buffer size and system state */
    sys.filter_size = 0xFF;
    filter_init();
    TEST_ASSERT_EQUAL_UINT8(5, filter_get_size());
    TEST_ASSERT_EQUAL_UINT8(5, sys.filter_size);
}

void test_filter_set_size_clamps_and_makes_odd(void) {
    /* filter_set_size should clamp to limits and force odd sizes */
    filter_set_size(8);   /* even > FILTER_MIN -> should become 9 (odd) */
    TEST_ASSERT_EQUAL_UINT8(9, filter_get_size());
    TEST_ASSERT_EQUAL_UINT8(9, sys.filter_size);

    filter_set_size(0);   /* below FILTER_MIN -> should become 1 */
    TEST_ASSERT_EQUAL_UINT8(1, filter_get_size());
}

void test_filter_median_and_rotation(void) {
    /* filter maintains ages and returns median of samples */
    filter_set_size(5);

    /* insert 5 values -> median should be 30 */
    filter_add_sample_and_get_median(10);
    filter_add_sample_and_get_median(30);
    filter_add_sample_and_get_median(20);
    filter_add_sample_and_get_median(50);
    uint16_t median = filter_add_sample_and_get_median(40);
    TEST_ASSERT_EQUAL_UINT16(30, median);

    /* next insert should replace the oldest (10) with 100 -> median becomes 40 */
    median = filter_add_sample_and_get_median(100);
    TEST_ASSERT_EQUAL_UINT16(40, median);
}

/* --- UART tests moved into this runner --- */
void test_uart_init_sets_ubrr_and_control_bits(void) {
    /* uart_init should compute UBRR and enable TX */
    uart_init(9600);
    TEST_ASSERT_TRUE(UCSR0B & (1 << TXEN0));
    TEST_ASSERT_TRUE(UCSR0C != 0);
}

void test_uart_putc_writes_to_udr_when_ready(void) {
    /* uart_putc should wait for UDRE and write character to UDR0 */
    UCSR0A = (1 << UDRE0);
    uart_putc('X');
    TEST_ASSERT_EQUAL_UINT8('X', UDR0);
}

void test_uart_print_uint16_outputs_last_digit_in_udr(void) {
    /* uart_print_uint16 should output ASCII digits; last digit ends in UDR0 */
    UCSR0A = (1 << UDRE0);
    uart_print_uint16(123);
    TEST_ASSERT_EQUAL_UINT8('3', UDR0);
}

void test_adc_isr_updates_sys_and_pwm(void) {
    /* ADC ISR should read ADCH into system volume and copy to OCR2B */
    ADCH = 0x5A;
    sys.adc_volume = 0;
    OCR2B = 0;
    ADC_vect(); /* simulate ISR */
    TEST_ASSERT_EQUAL_UINT8(0x5A, sys.adc_volume);
    TEST_ASSERT_EQUAL_UINT8(0x5A, OCR2B);
}

void test_buzzer_set_frequency_clamps_and_writes_ocr0a(void) {
    /* buzzer_set_frequency should clamp input and write OCR0A atomically */
    /* below min -> clamps to FREQ_MIN */
    OCR0A = 0xFF;
    buzzer_set_frequency(FREQ_MIN - 10);
    uint8_t ocr_min = OCR0A;

    /* above max -> clamps to FREQ_MAX */
    buzzer_set_frequency(FREQ_MAX + 1000);
    uint8_t ocr_max = OCR0A;

    TEST_ASSERT_NOT_EQUAL(ocr_min, 0xFF);
    TEST_ASSERT_NOT_EQUAL(ocr_max, 0xFF);
    TEST_ASSERT_TRUE(ocr_min != ocr_max);
}

void test_buttons_pcint_changes_filter_size(void) {
    /* PCINT ISR should detect button presses (active-low) and change filter */
    /* ensure filter in known state */
    filter_set_size(5);
    PIND = 0xFF; /* idle (pull-ups) */

    /* simulate press on PD4 (active-low) */
    PIND = ~(1 << PD4);
    PCINT2_vect();
    TEST_ASSERT_EQUAL_UINT8(3, filter_get_size()); /* 5 -> 3 */

    /* reset to 5 and press PD5 to increase */
    filter_set_size(5);
    PIND = ~(1 << PD5);
    PCINT2_vect();
    TEST_ASSERT_EQUAL_UINT8(7, filter_get_size()); /* 5 -> 7 */
}

void test_display_update_calls_draw_and_segment(void) {
    /* display_update should redraw only when values change and update segment */
    /* reset counters */
    hd_draw_calls = 0;
    twi_transmit_calls = 0;

    /* first call should draw both distance and freq and update segment */
    display_update(10, 300, 2);
    TEST_ASSERT_TRUE(hd_draw_calls > 0);
    TEST_ASSERT_TRUE(twi_transmit_calls > 0);

    /* calling with same values should not trigger redraw */
    int prev_hd = hd_draw_calls;
    int prev_twi = twi_transmit_calls;
    display_update(10, 300, 2);
    TEST_ASSERT_EQUAL(prev_hd, hd_draw_calls);
    TEST_ASSERT_EQUAL(prev_twi, twi_transmit_calls);
}

void test_ticks_to_distance_and_clamping(void) {
    /* ticks_to_distance_cm converts timer ticks to cm and clamps at MAX_DISTANCE_CM */
    TEST_ASSERT_EQUAL_UINT16(0, ticks_to_distance_cm(0));
    TEST_ASSERT_EQUAL_UINT16(1, ticks_to_distance_cm(116));
    /* near upper bound, large ticks should clamp */
    uint16_t big = 116 * (MAX_DISTANCE_CM + 10);
    TEST_ASSERT_EQUAL_UINT16(MAX_DISTANCE_CM, ticks_to_distance_cm(big));
}

/* UART edge cases ----------------------------------------------------- */

/* uart_print_uint16 should print '0' for zero and handle large values */
void test_uart_print_uint16_zero_and_max(void) {
    /* test zero prints '0' */
    UCSR0A = (1 << UDRE0);
    UDR0 = 0;
    uart_print_uint16(0);
    TEST_ASSERT_EQUAL_UINT8('0', UDR0);

    /* test max 65535 prints digits; last digit should be '5' */
    UCSR0A = (1 << UDRE0);
    UDR0 = 0;
    uart_print_uint16(65535);
    TEST_ASSERT_EQUAL_UINT8('5', UDR0);
}

/* uart_print on empty string should not write to UDR0 */
void test_uart_print_empty_string_no_write(void) {
    UCSR0A = 0; /* UDRE not set */
    UDR0 = 0x00;
    uart_print("");
    TEST_ASSERT_EQUAL_UINT8(0x00, UDR0);
}

/* Display clamp test -------------------------------------------------- */

/* display_update should clamp filter_value > 15 to 15 for segment map */
void test_display_clamps_filter_index(void) {
    twi_transmit_calls = 0;
    display_update(5, 300, 20); /* 20 > 15 -> should clamp to 15 */
    TEST_ASSERT_TRUE(twi_transmit_calls > 0);
}

/* Filter size=1 behavior ------------------------------------------------ */

/* When filter size is set to 1, the median should always be the single sample */
void test_filter_size_one_returns_inserted_sample(void) {
    filter_set_size(1);
    uint16_t v = filter_add_sample_and_get_median(77);
    TEST_ASSERT_EQUAL_UINT16(77, v);
}

void test_distance_to_frequency_mapping(void) {
    /* distance_to_frequency maps 0..MAX_DISTANCE_CM -> FREQ_MAX..FREQ_MIN */
    /* distance 0 -> max freq */
    TEST_ASSERT_EQUAL_UINT16(FREQ_MAX, distance_to_frequency(0));
    /* max distance -> min freq */
    TEST_ASSERT_EQUAL_UINT16(FREQ_MIN, distance_to_frequency(MAX_DISTANCE_CM));
    /* midpoint should be between min and max */
    uint16_t mid = MAX_DISTANCE_CM / 2;
    uint16_t fmid = distance_to_frequency(mid);
    TEST_ASSERT_TRUE(fmid <= FREQ_MAX && fmid >= FREQ_MIN);
}

void test_compute_ocr0a_for_freq_bounds(void) {
    /* compute_ocr0a_for_freq should produce values in 0..255 and differ for min/max */
    uint8_t o_min = compute_ocr0a_for_freq(FREQ_MIN);
    uint8_t o_max = compute_ocr0a_for_freq(FREQ_MAX);
    /* returns in 0..255 */
    TEST_ASSERT_TRUE(o_min <= 255);
    TEST_ASSERT_TRUE(o_max <= 255);
    /* ensure some difference between min and max */
    TEST_ASSERT_NOT_EQUAL(o_min, o_max);
}

/* Buzzer ISR test ------------------------------------------------------ */

/* The Timer0 Compare Match A ISR toggles the COM2B1 bit in TCCR2A to
   gate the PWM output. Calling the ISR twice should flip the bit twice. */
void test_buzzer_isr_toggles_pwm_gating(void) {
    /* start with COM2B1 cleared */
    TCCR2A &= ~(1 << COM2B1);
    TIMER0_COMPA_vect();
    /* first call should set COM2B1 */
    TEST_ASSERT_TRUE(TCCR2A & (1 << COM2B1));
    TIMER0_COMPA_vect();
    /* second call should clear COM2B1 */
    TEST_ASSERT_FALSE(TCCR2A & (1 << COM2B1));
}

/* ADC init test ------------------------------------------------------- */

/* adc_init should configure ADMUX for AVcc and left-adjust, enable ADC,
   start conversion, enable auto-trigger and ADC interrupt, and set prescaler. */
void test_adc_init_sets_control_bits(void) {
    /* clear registers first */
    ADMUX = 0;
    ADCSRA = 0;
    ADCSRB = 0;
    adc_init();
    /* ADMUX: REFS0 and ADLAR should be set */
    TEST_ASSERT_TRUE(ADMUX & (1 << REFS0));
    TEST_ASSERT_TRUE(ADMUX & (1 << ADLAR));
    /* ADCSRA: ADEN, ADSC, ADATE, ADIE should be set */
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADEN));
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADSC));
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADATE));
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADIE));
    /* prescaler bits ADPS2/1/0 should be set (128) */
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADPS2));
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADPS1));
    TEST_ASSERT_TRUE(ADCSRA & (1 << ADPS0));
    /* ADCSRB left as 0 in adc_init; check it's zero */
    TEST_ASSERT_EQUAL_UINT8(0, ADCSRB);
}

/* Ultrasonic ISR tests -------------------------------------------------- */

/* Rising capture: when ICES1 is set, ISR should reset TCNT1 and clear ICES1 */
void test_ultrasonic_capture_rising_resets_tcnt(void) {
    TCNT1 = 1234;
    TCCR1B |= (1 << ICES1); /* simulate configured for rising */
    TIMER1_CAPT_vect();
    TEST_ASSERT_EQUAL_UINT16(0, TCNT1);
    TEST_ASSERT_FALSE(TCCR1B & (1 << ICES1));
}

/* Falling capture: when ICES1=0, ISR should read ICR1 into sys and set flags */
void test_ultrasonic_capture_falling_sets_sys_values(void) {
    sys.us_busy = true;
    sys.new_distance_ready = false;
    /* make sure ICES1 is cleared to simulate falling edge */
    TCCR1B &= ~(1 << ICES1);
    ICR1 = 500;
    TIMER1_CAPT_vect();
    TEST_ASSERT_EQUAL_UINT16(500, sys.us_capture_value);
    TEST_ASSERT_TRUE(sys.new_distance_ready);
    TEST_ASSERT_FALSE(sys.us_busy);
    /* ICES1 should be set again to wait for next rising edge */
    TEST_ASSERT_TRUE(TCCR1B & (1 << ICES1));
}

/* Overflow ISR should mark timeout and clear busy flag */
void test_ultrasonic_overflow_sets_timeout(void) {
    sys.us_busy = true;
    sys.us_timeout = false;
    TIMER1_OVF_vect();
    TEST_ASSERT_TRUE(sys.us_timeout);
    TEST_ASSERT_FALSE(sys.us_busy);
}

/* buttons_init should configure PD4/PD5 as inputs with pull-ups and enable PCINT */
void test_buttons_init_sets_registers(void) {
    /* set DDRD to ones so clearing bits is observable */
    DDRD = 0xFF;
    PORTD = 0x00;
    PCICR = 0x00;
    PCMSK2 = 0x00;
    buttons_init();
    /* DDRD bits for PD4/PD5 should be cleared (inputs) */
    TEST_ASSERT_EQUAL_UINT8(0, DDRD & ((1 << PD4) | (1 << PD5)));
    /* PORTD pull-ups should be enabled for PD4/PD5 */
    TEST_ASSERT_TRUE(PORTD & ((1 << PD4) | (1 << PD5)));
    /* PCICR should have PCIE2 bit set */
    TEST_ASSERT_TRUE(PCICR & (1 << PCIE2));
    /* PCMSK2 should have PCINT20/21 bits set */
    TEST_ASSERT_TRUE(PCMSK2 & (1 << PCINT20));
    TEST_ASSERT_TRUE(PCMSK2 & (1 << PCINT21));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_filter_init_sets_defaults);
    RUN_TEST(test_filter_set_size_clamps_and_makes_odd);
    RUN_TEST(test_filter_median_and_rotation);
    /* UART tests */
    RUN_TEST(test_uart_init_sets_ubrr_and_control_bits);
    RUN_TEST(test_uart_putc_writes_to_udr_when_ready);
    RUN_TEST(test_uart_print_uint16_outputs_last_digit_in_udr);
    /* additional small-unit tests */
    RUN_TEST(test_adc_isr_updates_sys_and_pwm);
    RUN_TEST(test_buzzer_set_frequency_clamps_and_writes_ocr0a);
    RUN_TEST(test_buttons_pcint_changes_filter_size);
    RUN_TEST(test_display_update_calls_draw_and_segment);
    /* mapping and helper tests */
    RUN_TEST(test_ticks_to_distance_and_clamping);
    RUN_TEST(test_distance_to_frequency_mapping);
    RUN_TEST(test_compute_ocr0a_for_freq_bounds);

    /* buzzer ISR and ADC init tests */
    RUN_TEST(test_buzzer_isr_toggles_pwm_gating);
    RUN_TEST(test_adc_init_sets_control_bits);
    /* UART edge cases */
    RUN_TEST(test_uart_print_uint16_zero_and_max);
    RUN_TEST(test_uart_print_empty_string_no_write);
    /* Display clamp and filter size-1 tests */
    RUN_TEST(test_display_clamps_filter_index);
    RUN_TEST(test_filter_size_one_returns_inserted_sample);

    /* --- Ultrasonic ISR tests --- */
    RUN_TEST(test_ultrasonic_capture_rising_resets_tcnt);
    RUN_TEST(test_ultrasonic_capture_falling_sets_sys_values);
    RUN_TEST(test_ultrasonic_overflow_sets_timeout);
    /* buttons_init register test */
    RUN_TEST(test_buttons_init_sets_registers);
    return UNITY_END();
}
