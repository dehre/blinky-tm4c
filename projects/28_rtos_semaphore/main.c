//******************************************************************************
//
// Add blocking semaphores to the RTOS.
// Use them to implement a FIFO queue and to debounce a switch.
// See `os.h`, `semaphore-fifo.h`, and `switch-debounce.h`.
//
// For reference:
//   book "Real-Time Operating Systems for ARM Cortex-M Microcontrollers"
//   from page 193 onwards.
// Date: 03-01-2022
//
//******************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/debug.h>
#include "user-tasks.h"
#include "os.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

int main(void)
{
    OS_Init(THREADFREQ, userTask0);
    OS_ERRCHECK(OS_ThreadCreate(userTask1));
    OS_Launch();

    // This loop should not be reached.
    while (1)
        ;
}
