#include "test_helpers.h"

/* counters used by tests */
int twi_start_calls = 0;
int twi_transmit_calls = 0;
int hd_draw_calls = 0;

/* TWI stubs (simple counters) */
void TWI_MT_Start(void) { twi_start_calls++; }
void TWI_Transmit_SLAW(char a) { (void)a; twi_transmit_calls++; }
void TWI_Transmit_Byte(char b) { (void)b; twi_transmit_calls++; }
void TWI_Stop(void) { (void)0; }
void TWI_Init(void) { (void)0; }

/* HD44780 stubs matching lib header signatures */
char HD44780_PCF8574_Init(char addr) { (void)addr; return 0; }
void HD44780_PCF8574_DisplayOn(char addr) { (void)addr; }
void HD44780_PCF8574_DisplayClear(char addr) { (void)addr; }
char HD44780_PCF8574_PositionXY(char addr, char x, char y) { (void)addr; (void)x; (void)y; return 0; }
void HD44780_PCF8574_DrawString(char addr, char *s) { (void)addr; (void)s; hd_draw_calls++; }

/* Minimal cli/sei definitions for host tests */
void cli(void) { }
void sei(void) { }
