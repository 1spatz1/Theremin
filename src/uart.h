#ifndef UART_H
#define UART_H
#include <stdint.h>
void uart_init(uint32_t baud);
void uart_print(const char *s);
void uart_println(const char *s);
void uart_print_uint16(uint16_t val);
void uart_print_uint8(uint8_t val);
#endif
