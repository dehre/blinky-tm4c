#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inc/hw_memmap.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include "macro-utils.h"
#include "systick0.h"
#include "timer0.h"

#include "os.h"

//
// TCBState indicates whether the TCB can be used by OS_ThreadCreate
// to create a new thread.
//
enum TCBState
{
    TCBStateFree,
    TCBStateActive
};

//
// Thread Control Block
// IMPORTANT! The fn OSAsm_Start and OSAsm_ThreadSwitch, defined in os-asm.s,
//   expect the `sp` field to be placed first in the struct! Don't shuffle it!
//
typedef struct TCB
{
    int32_t *sp;          // pointer to stack (valid for threads not running)
    struct TCB *next;     // linked-list pointer
    uint32_t sleep;       // 0 means not sleeping
    enum TCBState status; // active or free
} TCB;

TCB tcbs[MAXNUMTHREADS];
size_t tcbsLen = sizeof(tcbs) / sizeof(TCB);
int32_t stacks[MAXNUMTHREADS][STACKSIZE];

// Pointer to the currently running thread.
TCB *runPt;

//
// The fn OS_Init sets the clock, then initializes SysTick and Timer0.
// Finally, it creates the first thread.
//
void OS_Init(uint32_t schedulerFrequencyHz, void (*firstTask)(void));

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
// The fn OSAsm_ThreadSwitch, defined in os-asm.s, is periodically called
//   by SysTick (ISR).
// It preemptively switches to the next thread, that is, it stores the stack
//   of the running thread and restores the stack of the next thread.
// It calls OS_Schedule to decide which thread is run next and update `runPt`.
//
extern void OSAsm_ThreadSwitch(void);

//
// The fn OS_Scheduler is called by OSAsm_ThreadSwitch and is responsible
//   for deciding the thread that is run next.
//
void OS_Scheduler(void);

//
// The fn OS_setInitialStack sets up the stack for a new thread as if it had
//   already been running and then suspended.
//
static void OS_setInitialStack(int32_t i);

//
// The fn OS_tcbsStatusInit initializes all TCBs' status to be free at startup.
//
static void OS_tcbsStatusInit(void);

// The flag firstThreadCreated indicates whether the first thread has been added
//   to the circular linked list of TCBs before the OS is launched.
static bool firstThreadCreated = false;

//
// The fn OS_FirstThreadCreate establishes the circular linked list of TCBs
//   with one node, and sets `runPt` to that node.
// The fn must be called before the OS is launched.
//
void OS_FirstThreadCreate(void (*task)(void));

//
// The fn OS_ThreadCreate adds a new thread to the circular linked list of TCBs,
//   then runs it. It fails if all the TCBs are already active.
// The fn can be called both:
//   * before the OS is launched (but after the first thread is created);
//   * after the OS is launched (by a running thread).
// The thread that calls this function keeps running until the end
//   of its scheduled time-slice. The new thread is run next.
//
OS_Err OS_ThreadCreate(void (*task)(void));

//
// The fn OS_ThreadKill kills the thread that calls it, then starts the thread
//   scheduled next. It fails if the last active thread tries to kill itself.
//
OS_Err OS_ThreadKill(void);

//
// The fn OS_ThreadSuspend halts the current thread and switches to the next.
// It's called by the running thread itself.
//
void OS_ThreadSuspend(void);

//
// The fn OS_ThreadSleep makes the current thread dormant for a specified time.
// It's called by the running thread itself.
// The fn OS_decrementTcbsSleepValue is called by Timer0 every ms and decrements
//   the value of `sleep` on the TCBs.
//
void OS_ThreadSleep(uint32_t ms);
static void OS_decrementTcbsSleepValue(void);

//*****************************************************************************
//
//       IMPLEMENTATION
//
//*****************************************************************************

void OS_Init(uint32_t schedulerFrequencyHz, void (*firstTask)(void))
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysTick0_Init(schedulerFrequencyHz, OSAsm_ThreadSwitch);
    Timer0_Init1KHz(OS_decrementTcbsSleepValue);
    OS_tcbsStatusInit();
    OS_FirstThreadCreate(firstTask);
}

void OS_Launch(void)
{
    ASSERT(firstThreadCreated);
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

static void OS_setInitialStack(int32_t i)
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

static void OS_tcbsStatusInit(void)
{
    for (uint32_t idx = 0; idx < MAXNUMTHREADS; idx++)
    {
        tcbs[idx].status = TCBStateFree;
    }
}

void OS_FirstThreadCreate(void (*task)(void))
{
    IntMasterDisable();
    tcbs[0].next = &(tcbs[0]);
    tcbs[0].sleep = 0;
    tcbs[0].status = TCBStateActive;

    OS_setInitialStack(0);
    stacks[0][STACKSIZE - 2] = (int32_t)task; // PC

    runPt = &(tcbs[0]); // thread 0 will run first
    firstThreadCreated = true;
    IntMasterEnable();
}

OS_Err OS_ThreadCreate(void (*task)(void))
{
    IntMasterDisable();
    uint32_t newTcbIdx;
    for (newTcbIdx = 0; newTcbIdx < MAXNUMTHREADS; newTcbIdx++)
    {
        if (tcbs[newTcbIdx].status == TCBStateFree)
            break;
    }
    if (newTcbIdx == MAXNUMTHREADS)
    {
        return OS_ERR_ALL_TCBS_ACTIVE;
    }

    tcbs[newTcbIdx].status = TCBStateActive;
    tcbs[newTcbIdx].sleep = 0;

    OS_setInitialStack(newTcbIdx);
    stacks[newTcbIdx][STACKSIZE - 2] = (int32_t)task; // PC

    tcbs[newTcbIdx].next = runPt->next;
    runPt->next = &(tcbs[newTcbIdx]);

    IntMasterEnable();
    return OS_ERR_NONE;
}

OS_Err OS_ThreadKill(void)
{
    if (runPt->next == runPt)
    {
        return OS_ERR_KILLING_LAST_ACTIVE_TCB;
    }

    IntMasterDisable();
    TCB *previousTcb = runPt;
    while (1)
    {
        previousTcb = previousTcb->next;
        if (previousTcb->next == runPt)
            break;
    }
    TCB *nextTcb = runPt->next;

    previousTcb->next = nextTcb;
    runPt->status = TCBStateFree;

    IntMasterEnable();
    OS_ThreadSuspend();

    // This line shouldn't be reached.
    // But it doesn't really matter, because the thread is killed anyway.
    return OS_ERR_NONE;
}

void OS_ThreadSuspend(void)
{
    SysTick0_ResetCounter();
    SysTick0_TriggerInterrupt();
}

void OS_ThreadSleep(uint32_t ms)
{
    runPt->sleep = ms;
    OS_ThreadSuspend();
}

static void OS_decrementTcbsSleepValue(void)
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
