#include <stdint.h>
#include <stdbool.h>

/* Minimal replacements for AVR registers and bit names used in host tests */
#ifndef AVR_IO_H_STUB
#define AVR_IO_H_STUB

/* basic bit macros */
#ifndef _BV
#define _BV(b) (1UL << (b))
#endif

/* Provide register/storage only when tests haven't provided their own
   stubs. Tests that declare registers should define AVR_IO_STUBS_IN_TEST
   before including or pulling in source files; this prevents duplicate
   definitions. If not defined, provide simple volatile storage for the
   common registers used by library translation units. */

#if !defined(AVR_IO_STUBS_IN_TEST)

/* Ports & DDR */
static volatile uint8_t DDRB = 0;
static volatile uint8_t PORTB = 0;
static volatile uint8_t DDRD = 0;
static volatile uint8_t PORTD = 0;
static volatile uint8_t PIND = 0;

/* Timer/Counter registers */
static volatile uint8_t TCCR2A = 0;
static volatile uint8_t TCCR2B = 0;
static volatile uint8_t OCR2B = 0;

static volatile uint8_t TCCR0A = 0;
static volatile uint8_t TCCR0B = 0;
static volatile uint8_t OCR0A = 0;
static volatile uint8_t TIMSK0 = 0;
/* Timer0 CTC bit (used in buzzer.c) */
#ifndef WGM01
#define WGM01 1
#endif
static volatile uint8_t TCCR1A = 0;
static volatile uint8_t TCCR1B = 0;
static volatile uint8_t TIMSK1 = 0;
static volatile uint8_t TIFR1 = 0;
static volatile uint16_t ICR1 = 0;
static volatile uint16_t TCNT1 = 0;

/* UART registers */
static volatile uint8_t UCSR0A = 0;
static volatile uint8_t UCSR0B = 0;
static volatile uint8_t UCSR0C = 0;
static volatile uint8_t UBRR0H = 0;
static volatile uint8_t UBRR0L = 0;
static volatile uint8_t UDR0 = 0;

/* TWI (I2C) registers */
static volatile uint8_t TWBR = 0;
static volatile uint8_t TWSR = 0;
static volatile uint8_t TWCR = 0;
static volatile uint8_t TWDR = 0;

/* ADC registers used by tests */
static volatile uint8_t ADMUX = 0;
static volatile uint8_t ADCSRA = 0;
static volatile uint8_t ADCSRB = 0;
static volatile uint8_t ADCH = 0;

/* Misc */
static volatile uint8_t SREG = 0;

/* Pin-change / PCINT */
static volatile uint8_t PCICR = 0;
static volatile uint8_t PCMSK2 = 0;

#endif /* !AVR_IO_STUBS_IN_TEST */

/* Bits (common AVR names used in project) */
#define PD3 3
#define PD4 4
#define PD5 5
#define PB0 0
#define PB1 1

/* ADC bit names used by adc.c */
#define REFS0 6
#define ADLAR 5
#define ADEN  7
#define ADSC  6
#define ADATE 5
#define ADIE  3
#define ADPS2 2
#define ADPS1 1
#define ADPS0 0

/* PCINT numbers for Port D on ATmega328P: PCINT16..23 map to PD0..PD7
   so PCINT20 is PD4, PCINT21 is PD5 */
#define PCINT20 4
#define PCINT21 5
#define PCIE2 2

/* Timer/compare bits (values chosen to match common AVR mapping) */
#define COM2B1 5
#define WGM21 1
#define WGM20 0
#define CS20 0
#define CS02 2
#define OCIE0A 1

/* Timer1 bits */
#define ICES1 6
#define CS11 1
#define ICF1 5
#define TOV1 0
#define ICIE1 5
#define TOIE1 0

/* UART bits */
#define TXEN0 3
#define UCSZ01 2
#define UCSZ00 1
#define UDRE0 5

/* TWI bits */
#define TWEN 2
#define TWINT 7
#define TWSTA 5
#define TWEA 6
#define TWSTO 4

/* Provide aliases that some headers expect (TWI_* macros in twi.h)
   so code that uses TWI_TWBR etc will compile in native tests. */
#ifndef TWI_TWBR
#define TWI_TWBR TWBR
#endif
#ifndef TWI_TWSR
#define TWI_TWSR TWSR
#endif
#ifndef TWI_TWCR
#define TWI_TWCR TWCR
#endif
#ifndef TWI_TWDR
#define TWI_TWDR TWDR
#endif

#endif /* AVR_IO_H_STUB */