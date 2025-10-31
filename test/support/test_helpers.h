/* test_helpers.h
 * Shared test helper stubs (TWI and HD44780) used by host unit tests.
 * Included from test TUs (path: test/support/test_helpers.h)
 */
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdint.h>

/* counters to assert calls in tests */
extern int twi_start_calls;
extern int twi_transmit_calls;
extern int hd_draw_calls;

/* TWI stubs (simple counters) */
void TWI_MT_Start(void);
void TWI_Transmit_SLAW(char a);
void TWI_Transmit_Byte(char b);
void TWI_Stop(void);
void TWI_Init(void);

/* HD44780 stubs matching lib header signatures */
char HD44780_PCF8574_Init(char addr);
void HD44780_PCF8574_DisplayOn(char addr);
void HD44780_PCF8574_DisplayClear(char addr);
char HD44780_PCF8574_PositionXY(char addr, char x, char y);
void HD44780_PCF8574_DrawString(char addr, char *s);

/* Minimal cli/sei definitions for host tests */
void cli(void);
void sei(void);

#endif // TEST_HELPERS_H
