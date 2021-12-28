//******************************************************************************
//
// Add dynamic thread creation and killing to the RTOS.
//
// Creating and killing threads requires a mechanism to dynamically allocate
//   and deallocate TCBs and stack areas.
// We could build a memory manager and use it to allocate and deallocate.
// However, if the maximum number of threads is small, we can implement a
//   simple fixed allocation scheme to manage TCBs and stack areas.
// With this OS we allocate `MAXNUMTHREADS` at compile time, and use a `status`
//   field in the TCB to signify if the TCB/stack is active or free.
//
// The program can be tried by connecting PE0, PE1, and PE2 to a logic analyzer.
//
// For reference:
//   book "Real-Time Operating Systems for ARM Cortex-M Microcontrollers" page 191.
// Date: 28-12-2021
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
