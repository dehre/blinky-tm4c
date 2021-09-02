//*****************************************************************************
//
// Utility functions for reading the value of the onboard switch SW1 on PF4.
// SW1 uses negative logic and is connected to the internal pull-up resistor.
// To facilitate its usage, however, the function `OnboardSw1_Read()`
//     reads `1` if the switch is pressed, or `0` otherwise.
//
// Usage:
// ```c
// OnboardSw1_Init();
// uint32_t isPressed = OnboardSw1_Read();
// ```
//
//*****************************************************************************

#ifndef _ONBOARD_SW1_H_
#define _ONBOARD_SW1_H_

#include <stdint.h>

void OnboardSw1_Init(void);

uint32_t OnboardSw1_Read(void);

#endif
