#include <avr/io.h>
#include "../include/uart.h"

void uart_init(uint32_t baud) {
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_print(const char *s) {
    while (*s) uart_putc(*s++);
}
void uart_println(const char *s) { uart_print(s); uart_print("\r\n"); }

void uart_print_uint16(uint16_t val) {
    char buf[6];
    int i=0;
    if (val==0) { uart_putc('0'); return; }
    while (val>0 && i<6) { buf[i++] = '0' + (val % 10); val/=10; }
    while (i--) uart_putc(buf[i]);
}
void uart_print_uint8(uint8_t v) { uart_print_uint16(v); }
