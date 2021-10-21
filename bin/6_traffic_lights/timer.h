#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <stdint.h>

//
// Configure and enable the SysTick counter for 100ms.
// The clock is assumed to run at 80MHz.
//
void TimerSystickSetup(void);

void TimerReset(void);

void TimerWait1s(uint8_t times);

#endif
