//******************************************************************************
//
// Implement a priority scheduler.
// Then run an event thread as high priority main thread.
// In short:
//   * the event thread blocks waiting for a semaphore;
//   * the ISR signals the semaphore and suspends the current main thread;
//   * because the event thread has high priority and isn't blocked anymore,
//       the scheduler runs it immediately.
//
// The program can be tried by connecting a positive logic switch to PB6.
// The event thread blinks the onboard red LED on PF1.
// The main threads can be seen blocking by connecting PE0, PE1, and PE2
//   to a logic analyzer.
//
// Changes in `os.h`, `gpiopb6-signal.h`, `user-tasks.h`, and `blinky.h`.
//
// For reference:
//   book "Real-Time Operating Systems for ARM Cortex-M Microcontrollers"
//   from page 237 to page 240.
// Circuit's diagram available in this repository.
// Date: 04-01-2022
//
//******************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/debug.h>
#include "os.h"
#include "user-tasks.h"
#include "gpiopb6-signal.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

int main(void)
{
    //
    // Initialize OS and threads.
    //
    OS_Init(THREADFREQ, userTask0, 5, "userTask0");
    OS_ERRCHECK(OS_ThreadCreate(userTask1, 5, "userTask1"));
    OS_ERRCHECK(OS_ThreadCreate(userTaskOnPB6RisingEdge, 3, "userTaskOnPB6RisingEdge"));

    //
    // Initialize other resources.
    //
    GPIOPB6_Signal_Init();

    //
    // Launch OS.
    //
    OS_Launch();

    // This loop should not be reached.
    while (1)
        ;
}
