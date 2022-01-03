#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include "macro-utils.h"

#include "timer0.h"

void Timer0_Init1KHz(void (*periodicIntHandler)(void))
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000);
    TimerIntRegister(TIMER0_BASE, TIMER_A, periodicIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer0_Enable(void)
{
    TimerEnable(TIMER0_BASE, TIMER_A);
}
