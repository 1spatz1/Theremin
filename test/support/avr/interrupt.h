/* File: `test/support/avr/interrupt.h` */
#ifndef AVR_INTERRUPT_H_STUB
#define AVR_INTERRUPT_H_STUB

/* ISR macro used by sources: expand to a normal function */
#ifndef ISR
#define ISR(vec) void vec(void)
#endif

/* cli/sei used by buzzer/other modules */
#ifndef cli
/* declare only; tests provide definitions where needed */
void cli(void);
#endif
#ifndef sei
/* declare only; tests provide definitions where needed */
void sei(void);
#endif

#endif /* AVR_INTERRUPT_H_STUB */