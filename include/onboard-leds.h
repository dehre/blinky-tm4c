//*****************************************************************************
//
// Utility functions for using the onboard LEDs on PF1, PF2, and PF3.
// For blinking, the parameter `blinkDelayCount` specifies the number
//  of CPU cycles between LED toggles.
//
// Usage:
// ```c
// #include "onboard-leds.h"
//
// OnboardLEDs_Init(SysCtlClockGet() / 2); // half a second between LED toggles
// OnboardLEDs_RedBlink();
// ```
//
//*****************************************************************************

#ifndef _ONBOARD_LEDS_H_
#define _ONBOARD_LEDS_H_

#include <stdint.h>

void OnboardLEDs_Init(uint32_t blinkDelayCount);

void OnboardLEDs_RedBlink(void);

void OnboardLEDs_BlueBlink(void);

void OnboardLEDs_GreenBlink(void);

#endif
