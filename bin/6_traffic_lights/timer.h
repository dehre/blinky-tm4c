
#pragma once
#include <stdint.h>

//
// Configure and enable the SysTick counter for 100ms.
// The clock is assumed to run at 80MHz.
//
static void TimerSystickSetup(void);

static void TimerReset(void);

static void TimerWait1s(uint8_t times);
