#ifndef OS_H_INCLUDED
#define OS_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/debug.h>

#define MAXNUMTHREADS 10 // maximum number of threads
#define STACKSIZE 100    // number of 32-bit words in stack
#define THREADFREQ 1000  // maximum time-slice before the scheduler is run, in Hz

typedef enum OS_Err
{
    OS_ERR_NONE = 0,
    OS_ERR_ALL_TCBS_ACTIVE,
    OS_ERR_KILLING_LAST_ACTIVE_TCB,
} OS_Err;

#define OS_ERRCHECK(expr)              \
    if (expr != OS_ERR_NONE)           \
    {                                  \
        __error__(__FILE__, __LINE__); \
    }

void OS_Init(uint32_t schedulerFrequencyHz, void (*firstTask)(void));
void OS_Launch(void);
OS_Err OS_ThreadCreate(void (*task)(void));
OS_Err OS_ThreadKill(void);
void OS_ThreadSuspend(void);
void OS_ThreadSleep(uint32_t ms);
void OS_SemaphoreWait(int32_t *s);
void OS_SemaphoreSignal(int32_t *s);

#endif
