#include "timer.h"
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

static volatile bool _100msElapsed;

static void sysTickInterruptHandler(void)
{
    _100msElapsed = true;
}

//
// Configure and enable the SysTick counter for 100ms.
// The clock is set to run at 80MHz.
//
void TimerSystickSetup(void)
{
    //
    // Set the clocking to run directly from the crystal at 80MHz.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysTickPeriodSet(8000000);
    SysTickIntRegister(sysTickInterruptHandler);
    SysTickEnable();
}

void TimerReset(void)
{
    _100msElapsed = false;
}

void TimerWait1s(uint8_t times)
{
    volatile uint16_t ui16Loop;
    for (ui16Loop = 0; ui16Loop < 10 * times; ui16Loop++)
    {
        while (!_100msElapsed)
            ;
        _100msElapsed = false;
    }
}
