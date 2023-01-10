//******************************************************************************
//
// A barebone RTOS running on the Tiva LaunchPad.
// In short:
//   * Three hardcoded threads are linked together in a circular queue and their
//     stack initialized with dummy values.
//   * The first thread's stack is "restored" on the main stack and run.
//   * The SysTick ISR triggers thread switching, which saves the stack of
//     the current thread and restores the one of the next thread.
//     The scheduler's algorithm decides which one is the next thread.
//   * The Timer0 ISR decrements the value of `sleep` in the
//     thread-control-blocks every ms.
//   * OS_Suspend triggers thread switching without waiting for SysTick,
//     OS_Sleep updates the value of `sleep` for the current thread-control-block.
//
// The program can be tried by connecting PE0, PE1, and PE2 to a logic analyzer.
//
// For reference: book "Real-Time Operating Systems for ARM Cortex-M Microcontrollers"
//   from page 170 onwards.
// Date: 23-12-2021
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
    OS_AddThreads(userTask0, userTask1, userTask2);
    OS_Init(THREADFREQ);
    OS_Launch();

    // This loop should not be reached.
    while (1)
        ;
}
