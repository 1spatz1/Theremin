#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "../lib/twi/twi.h"
#include "../lib/hd44780pcf8574/hd44780pcf8574.h"
#include "display.h"

#define LCD_ADDR       0x27
#define SEGMENT_ADDR   0x21

// 7-segment bit map (0–F, common cathode)
static const uint8_t SEGMENT_MAP[16] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // b
    0b00111001, // C
    0b01011110, // d
    0b01111001, // E
    0b01110001  // F
};

static uint16_t prev_distance = 0;
static uint16_t prev_freq = 0;
static uint8_t  prev_filter = 0xFF;

// helper: write to the 7-segment expander
static void segment_write(uint8_t value)
{
    TWI_MT_Start();
    TWI_Transmit_SLAW(SEGMENT_ADDR);
    TWI_Transmit_Byte(value);
    TWI_Stop();
}

// initialize both displays
void display_init(void)
{
    TWI_Init();
    HD44780_PCF8574_Init(LCD_ADDR);
    HD44780_PCF8574_DisplayOn(LCD_ADDR);
    HD44780_PCF8574_DisplayClear(LCD_ADDR);

    HD44780_PCF8574_PositionXY(LCD_ADDR, 0, 0);
    HD44780_PCF8574_DrawString(LCD_ADDR, "Theremin Ready");

    // small 7-seg test
    for (uint8_t i = 0; i < 16; i++) {
        segment_write(~SEGMENT_MAP[i]); // invert if active low
        _delay_ms(100);
    }
    segment_write(~SEGMENT_MAP[0]);
    _delay_ms(200);
    segment_write(~SEGMENT_MAP[15]); // show F as idle
}

// main display update
void display_update(uint16_t distance_cm, uint16_t freq_hz, uint8_t filter_value)
{
    char buffer[17];

    // --- Update LCD only when values changed ---
    if (distance_cm != prev_distance) {
        HD44780_PCF8574_PositionXY(LCD_ADDR, 0, 0);
        snprintf(buffer, sizeof(buffer), "Distance:%3dcm ", distance_cm);
        HD44780_PCF8574_DrawString(LCD_ADDR, buffer);
        prev_distance = distance_cm;
    }

    if (freq_hz != prev_freq) {
        HD44780_PCF8574_PositionXY(LCD_ADDR, 0, 1);
        snprintf(buffer, sizeof(buffer), "Freq:%5dHz ", freq_hz);
        HD44780_PCF8574_DrawString(LCD_ADDR, buffer);
        prev_freq = freq_hz;
    }

    // --- 7-segment filter display ---
    if (filter_value != prev_filter) {
        if (filter_value > 15) filter_value = 15;
        segment_write(~SEGMENT_MAP[filter_value]);
        prev_filter = filter_value;
    }
}

