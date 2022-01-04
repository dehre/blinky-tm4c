#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include "instrument-trigger.h"
#include "os.h"
#include "gpiopb6-signal.h"

#include "user-tasks.h"

InstrumentTrigger_Create(E, 0);
void userTask0(void)
{
    InstrumentTriggerPE0_Init();
    uint32_t count = 0;
    while (1)
    {
        count++;
        InstrumentTriggerPE0_Toggle();
        SysCtlDelay(100);

        if (count == 5000)
        {
            OS_ERRCHECK(OS_ThreadCreate(userTask2, 5, "userTask2"));
        }

        if (count == 10000)
        {
            OS_ERRCHECK(OS_ThreadKill());
        }
    }
}

InstrumentTrigger_Create(E, 1);
void userTask1(void)
{
    InstrumentTriggerPE1_Init();
    while (1)
    {
        for (uint32_t count = 0; count < 14; count++)
        {
            InstrumentTriggerPE1_Toggle();
            SysCtlDelay(100);
        }
        OS_ThreadSuspend();
    }
}

InstrumentTrigger_Create(E, 2);
void userTask2(void)
{
    InstrumentTriggerPE2_Init();
    uint32_t count = 0;
    while (1)
    {
        count++;
        if (count % 125 == 0)
        {
            OS_ThreadSleep(50);
        }

        InstrumentTriggerPE2_Toggle();
        SysCtlDelay(100);
    }
}

InstrumentTrigger_Create(F, 1);
void userTaskOnPB6RisingEdge(void)
{
    uint32_t clockRate = SysCtlClockGet();
    InstrumentTriggerPF1_Init();
    while (1)
    {
        OS_SemaphoreWait(&GPIOPB6_Signal_RisingEdgeHit);

        // some long-running task
        for (uint32_t idx = 0; idx < 4; idx++)
        {
            InstrumentTriggerPF1_Toggle();
            SysCtlDelay(clockRate / 12);
        }
    }
}
