#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include "instrument-trigger.h"
#include "os.h"

#include "user-tasks.h"

InstrumentTrigger_Create(E, 0);
void userTask0(void)
{
    static bool gpioInitialized = false;
    if (!gpioInitialized)
    {
        InstrumentTriggerPE0_Init();
        gpioInitialized = true;
    }

    uint32_t count;
    while (1)
    {
        count++;
        InstrumentTriggerPE0_Toggle();
        SysCtlDelay(100);
    }
}

InstrumentTrigger_Create(E, 1);
void userTask1(void)
{
    static bool gpioInitialized = false;
    if (!gpioInitialized)
    {
        InstrumentTriggerPE1_Init();
        gpioInitialized = true;
    }

    while (1)
    {
        for (uint32_t count = 0; count < 14; count++)
        {
            InstrumentTriggerPE1_Toggle();
            SysCtlDelay(100);
        }
        OS_Suspend();
    }
}

InstrumentTrigger_Create(E, 2);
void userTask2(void)
{
    static bool gpioInitialized = false;
    if (!gpioInitialized)
    {
        InstrumentTriggerPE2_Init();
        gpioInitialized = true;
    }

    uint32_t count;
    while (1)
    {
        count++;
        if (count % 125 == 0)
        {
            OS_Sleep(50);
        }

        InstrumentTriggerPE2_Toggle();
        SysCtlDelay(100);
    }
}
