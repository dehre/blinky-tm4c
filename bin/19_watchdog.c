//***************************************************************************************************
//
// Use the watchdog as a simple heartbeat for the system.
// Each time the watchdog is fed, the onboard LED is toggled.
// If the watchdog is not periodically fed, it resets the system.
// To stop the watchdog being fed, press the onboard switch SW1.
//
// Date: 08-11-2021
//
//***************************************************************************************************

//***************************************************************************************************
// OTHER IMPORTANT INFORMATION
//***************************************************************************************************
//
// The watchdog timer module generates the first timeout signal when the 32-bit
// counter reaches the zero state after being enabled; enabling the counter also
// enables the watchdog timer interrupt. After the first timeout event, the 32-bit
// counter is reloaded with the value of the watchdog timer load register, and
// the timer resumes counting down from that value. If the timer counts down to
// its zero state again before the first timeout interrupt is cleared, and the
// reset signal has been enabled, the watchdog timer asserts its reset signal to
// the system. If the interrupt is cleared before the 32-bit counter reaches its
// second timeout, the 32-bit counter is loaded with the value in the load register,
// and counting resumes from that value.
//
//***************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/watchdog.h"
#include "heart-beat.h"
#include "onboard-sw1.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

static volatile bool FeedWatchdog = true;
static void Watchdog_IntHandler(void);
static void OnboardSw1_Pressed(void);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();
    OnboardSw1_Init();
    UART_Init();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_WDOG0))
        ;
    WatchdogReloadSet(WATCHDOG0_BASE, SysCtlClockGet());
    WatchdogIntRegister(WATCHDOG0_BASE, Watchdog_IntHandler);
    WatchdogResetEnable(WATCHDOG0_BASE);
    WatchdogStallEnable(WATCHDOG0_BASE); // Stop counting when the processor is stopped by the debugger
    WatchdogEnable(WATCHDOG0_BASE);

    if (SysCtlResetCauseGet() & SYSCTL_CAUSE_WDOG0)
    {
        SysCtlResetCauseClear(SYSCTL_CAUSE_WDOG0);
        UARTprintf("Application restarted after a watchdog timer reset\n");
    }
    else
    {
        UARTprintf("Application started\n");
    }

    while (1)
    {
        if (OnboardSw1_Read() == 1)
        {
            OnboardSw1_Pressed();
            while (1)
                ;
        }
    }
}

void Watchdog_IntHandler(void)
{
    if (!FeedWatchdog)
        return;

    WatchdogIntClear(WATCHDOG0_BASE);
    HeartBeat_Toggle();
}

void OnboardSw1_Pressed(void)
{
    UARTprintf("Starving watchdog, system will reset...\n");
    FeedWatchdog = false;
}
