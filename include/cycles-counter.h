//*****************************************************************************
//
// Utility functions to collect the number of elapsed clock cycles
// between function calls. It uses SysTick.
//
// Usage:
// ```c
// uint32_t clockCyclesDump[200] = {};
//
// CyclesCounter_InitSysTick();
// CyclesCounter_Reset();
//
// while (1)
// {
//     DoSomething();
//     CyclesCounter_Push(clockCyclesDump, sizeof(clockCyclesDump) / sizeof(uint32_t));
// }
// ```
//
//*****************************************************************************

#ifndef _CYCLES_COUNTER_H_
#define _CYCLES_COUNTER_H_

#include <stdint.h>

//
// Initialize SysTick.
//
void CyclesCounter_InitSysTick(void);

//
// Reset the counter.
//
void CyclesCounter_Reset(void);

//
// Collect the number of clock cycles elapsed from the last
// call to `CyclesCounter_Push` or `CyclesCounter_Reset`.
// Returns 1 if the buffer is full.
//
uint8_t CyclesCounter_Push(uint32_t buffer[], uint32_t bufferLen);

#endif
