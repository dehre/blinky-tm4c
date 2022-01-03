#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>

#include "systick0.h"

void SysTick0_Init(uint32_t frequencyHz, void (*periodicIntHandler)(void))
{
    uint32_t clockRate = SysCtlClockGet();
    SysTickPeriodSet((clockRate / frequencyHz) - 1);
    SysTickIntRegister(periodicIntHandler);
    SysTick0_ResetCounter();
}

void SysTick0_Enable(void)
{
    SysTickEnable();
}

void SysTick0_ResetCounter(void)
{
    // Any write to this register clears the SysTick counter to 0.
    // See the microcontroller's data sheet page 123.
    NVIC_ST_CURRENT_R = 0;
}

void SysTick0_TriggerInterrupt(void)
{
    // On a write, changes the SysTick exception state to pending.
    // See the microcontroller's data sheet page 161.
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PENDSTSET;
}
