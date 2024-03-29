#ifndef OS_H_INCLUDED
#define OS_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define NUMTHREADS 3    // maximum number of threads
#define STACKSIZE 100   // number of 32-bit words in stack
#define THREADFREQ 1000 // maximum time-slice before the scheduler is run, in Hz

int32_t OS_AddThreads(
    void (*task0)(void),
    void (*task1)(void),
    void (*task2)(void));

void OS_Init(uint32_t schedulerFrequencyHz);
void OS_Launch(void);
void OS_Suspend(void);
void OS_Sleep(uint32_t ms);

#endif
