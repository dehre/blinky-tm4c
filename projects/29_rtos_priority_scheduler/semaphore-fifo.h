//*****************************************************************************
//
// FIFO queue used to safely pass data from multiple producer threads to
//   multiple consumer threads.
// Producers will suspend (`OS_SemaphoreWait`) when the FIFO is full, and
//   consumers will suspend (`OS_SemaphoreWait`) when the FIFO is empty.
//
// Usage:
// ```c
// #include "semaphore-fifo.h"
//
// SemaphoreFifo_Init();
// SemaphoreFifo_Put(123);
// uint32_t data = SemaphoreFifo_Get();
// ```
//
//*****************************************************************************

#ifndef SEMAPHORE_FIFO_H_INCLUDED
#define SEMAPHORE_FIFO_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define FIFO_SIZE 10

void SemaphoreFifo_Init(void);
void SemaphoreFifo_Put(uint32_t data);
uint32_t SemaphoreFifo_Get(void);

#endif
