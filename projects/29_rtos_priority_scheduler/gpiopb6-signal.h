//*****************************************************************************
//
// Set up GPIO PB6 to signal the semaphore `GPIOPB6_Signal_RisingEdgeHit`
//   on rising edges.
//
//*****************************************************************************

#ifndef GPIOPB6_SIGNAL_H_INCLUDED
#define GPIOPB6_SIGNAL_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

extern int32_t GPIOPB6_Signal_RisingEdgeHit;

void GPIOPB6_Signal_Init(void);

#endif
