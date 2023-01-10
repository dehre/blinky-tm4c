#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/timer.h>
#include "macro-utils.h"
#include "systick0.h"
#include "timer0.h"

#include "os.h"

//
// Thread Control Block
//
typedef struct TCB
{
    int32_t *sp;      // pointer to stack (valid for threads not running
    struct TCB *next; // linked-list pointer
    uint32_t sleep;   // 0 means not sleeping
} TCB;

TCB tcbs[NUMTHREADS];
size_t tcbsLen = sizeof(tcbs) / sizeof(TCB);
int32_t stacks[NUMTHREADS][STACKSIZE];

TCB *runPt; // pointer to the currently running thread

//
// The fn OS_SetInitialStack and OS_AddThreads link together the threads in
//   a circular list and set up the stack for each of them as if they had
//   already been running and then suspended.
//
static void OS_SetInitialStack(int32_t i);
int32_t OS_AddThreads(void (*task0)(void), void (*task1)(void), void (*task2)(void));

//
// The fn OS_Init sets the clock, then initializes SysTick and Timer0.
//
void OS_Init(uint32_t schedulerFrequencyHz);

//
// The fn OS_Launch enables SysTick and Timer0, then calls OSAsm_Start,
//   which starts the first thread.
//
void OS_Launch(void);

//
// The fn OSAsm_Start, defined in os-asm.s, is called by OS_Launch once.
// It "restores" the first thread's stack on the main stack.
//
extern void OSAsm_Start(void);

//
// The fn OSAsm_ThreadSwitch, defined in os-asm.s, is periodically called by SysTick (ISR).
// It preemptively switches to the next thread, that is, it stores the stack of the running
//   thread and restores the stack of the next thread.
// It calls OS_Schedule to decide which thread is run next and update `runPt`.
//
extern void OSAsm_ThreadSwitch(void);

//
// The fn OS_Scheduler is called by OSAsm_ThreadSwitch and is responsible for deciding
//   the thread that is run next.
//
void OS_Scheduler(void);

//
// The fn OS_Suspend halts the current thread and switches to the next.
// It's called by the running thread itself.
//
void OS_Suspend(void);

//
// The fn OS_Sleep makes the current thread dormant for a specified time.
// It's called by the running thread itself.
// The fn OS_DecrementTcbsSleepValue is called by Timer0 every ms and decrements the
//   the value of `sleep` on the thread-control-blocks.
//
void OS_Sleep(uint32_t ms);
static void OS_DecrementTcbsSleepValue(void);

//*****************************************************************************
//
//       IMPLEMENTATION
//
//*****************************************************************************

static void OS_SetInitialStack(int32_t i)
{
    tcbs[i].sp = &stacks[i][STACKSIZE - 16]; // thread stack pointer

    stacks[i][STACKSIZE - 1] = 0x01000000; // thumb bit (PSR)
    // stacks[i][STACKSIZE - 2] =           // R15 (PC) -> set later in fn OS_AddThreads
    stacks[i][STACKSIZE - 3] = 0x14141414;  // R14 (LR)
    stacks[i][STACKSIZE - 4] = 0x12121212;  // R12
    stacks[i][STACKSIZE - 5] = 0x03030303;  // R3
    stacks[i][STACKSIZE - 6] = 0x02020202;  // R2
    stacks[i][STACKSIZE - 7] = 0x01010101;  // R1
    stacks[i][STACKSIZE - 8] = 0x00000000;  // R0
    stacks[i][STACKSIZE - 9] = 0x11111111;  // R11
    stacks[i][STACKSIZE - 10] = 0x10101010; // R10
    stacks[i][STACKSIZE - 11] = 0x09090909; // R9
    stacks[i][STACKSIZE - 12] = 0x08080808; // R8
    stacks[i][STACKSIZE - 13] = 0x07070707; // R7
    stacks[i][STACKSIZE - 14] = 0x06060606; // R6
    stacks[i][STACKSIZE - 15] = 0x05050505; // R5
    stacks[i][STACKSIZE - 16] = 0x04040404; // R4
}

int32_t OS_AddThreads(
    void (*task0)(void),
    void (*task1)(void),
    void (*task2)(void))
{
    IntMasterDisable();
    tcbs[0].next = &(tcbs[1]);
    tcbs[1].next = &(tcbs[2]);
    tcbs[2].next = &(tcbs[0]);

    OS_SetInitialStack(0);
    stacks[0][STACKSIZE - 2] = (int32_t)task0; // PC
    OS_SetInitialStack(1);
    stacks[1][STACKSIZE - 2] = (int32_t)task1; // PC
    OS_SetInitialStack(2);
    stacks[2][STACKSIZE - 2] = (int32_t)task2; // PC

    runPt = &(tcbs[0]); // thread 0 will run first
    IntMasterEnable();
    return 1;
}

void OS_Init(uint32_t schedulerFrequencyHz)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysTick0_Init(schedulerFrequencyHz, OSAsm_ThreadSwitch);
    Timer0_Init1KHz(OS_DecrementTcbsSleepValue);
}

void OS_Launch(void)
{
    SysTick0_Enable();
    Timer0_Enable();
    OSAsm_Start();
}

void OS_Scheduler(void)
{
    runPt = runPt->next; // Round Robin
    while (runPt->sleep)
    {
        runPt = runPt->next;
    }
}

void OS_Suspend(void)
{
    SysTick0_ResetCounter();
    SysTick0_TriggerInterrupt();
}

void OS_Sleep(uint32_t ms)
{
    runPt->sleep = ms;
    OS_Suspend();
}

static void OS_DecrementTcbsSleepValue(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    for (size_t idx = 0; idx < tcbsLen; idx++)
    {
        if (tcbs[idx].sleep > 0)
        {
            tcbs[idx].sleep -= 1;
        }
    }
}
