/* Minimal pgmspace.h replacement for host/unit tests */
#ifndef AVR_PGMSPACE_H_STUB
#define AVR_PGMSPACE_H_STUB

#include <stdint.h>

/* PROGMEM used to mark data stored in program space on AVR; for native
   tests we can ignore it. */
#define PROGMEM

/* simple helpers that mimic pgmspace accessors */
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#define pgm_read_word(addr) (*(const uint16_t *)(addr))

#endif /* AVR_PGMSPACE_H_STUB */
