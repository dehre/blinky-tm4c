#ifndef SYSTICK0_H_INCLUDED
#define SYSTICK0_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

void SysTick0_Init(uint32_t frequencyHz, void (*periodicIntHandler)(void));
void SysTick0_Enable(void);
void SysTick0_ResetCounter(void);
void SysTick0_TriggerInterrupt(void);

#endif
