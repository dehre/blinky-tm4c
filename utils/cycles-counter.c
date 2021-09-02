#include <stdint.h>
#include <driverlib/systick.h>
#include "inc/tm4c123gh6pm.h"

#include "cycles-counter.h"

#define SYSTICK_PERIOD_MAX 16777216

void CyclesCounter_InitSysTick(void)
{
    SysTickPeriodSet(SYSTICK_PERIOD_MAX);
    SysTickEnable();
}

void CyclesCounter_Reset(void)
{
    // Any write to this register clears the SysTick counter to 0.
    NVIC_ST_CURRENT_R = 0;
}

uint8_t CyclesCounter_Push(uint32_t buffer[], uint32_t bufferLen)
{
    static uint32_t idx = 0;
    if (idx > bufferLen)
        return 1;

    buffer[idx] = SYSTICK_PERIOD_MAX - SysTickValueGet();
    idx++;

    CyclesCounter_Reset();
    return 0;
}
