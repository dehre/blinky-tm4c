#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/watchdog.h"

#include "watchdog-init.h"

void Watchdog_Init(uint32_t loadValue, void (*userfn)(void));

static volatile bool FeedWatchdog = true;
static void interruptHandler(void);
static void (*customDefinedFn)(void);

void Watchdog_Init(uint32_t loadValue, void (*recurringFn)(void))
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_WDOG0))
        ;

    WatchdogReloadSet(WATCHDOG0_BASE, loadValue);
    WatchdogIntRegister(WATCHDOG0_BASE, interruptHandler);
    WatchdogResetEnable(WATCHDOG0_BASE);

    WatchdogStallEnable(WATCHDOG0_BASE); // Stop counting when the processor is stopped by the debugger
    WatchdogEnable(WATCHDOG0_BASE);

    customDefinedFn = recurringFn;
}

void interruptHandler(void)
{
    if (!FeedWatchdog)
        return;

    WatchdogIntClear(WATCHDOG0_BASE);
    customDefinedFn();
}
