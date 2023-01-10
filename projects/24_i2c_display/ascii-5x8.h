#ifndef ASCII_5X8_H_INCLUDED
#define ASCII_5X8_H_INCLUDED

#include <stdint.h>

#define ASCII_5X8_WIDTH 5
#define ASCII_5X8_HEIGHT 8
#define ASCII_5X8_MIN_VALUE 0x20
#define ASCII_5X8_MAX_VALUE 0x7F

// This table contains the hex values that represent pixels
// for a font that is 5 pixels wide and 8 pixels high
extern const uint8_t ASCII_5X8[][ASCII_5X8_WIDTH];

#endif
