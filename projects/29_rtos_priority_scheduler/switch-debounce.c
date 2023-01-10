#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include "macro-utils.h"
#include "os.h"

#include "switch-debounce.h"

#define PERIPHERAL SYSCTL_PERIPH_GPIOB
#define PORT GPIO_PORTB_BASE
#define PIN GPIO_PIN_5

static int32_t interruptHit = 0;
static void (*onTouch)(void);
static void (*onRelease)(void);
static void risingFallingEdgeIntHandler(void);

void SwitchDebouncePB5_Init(void (*callbackOnTouch)(void), void (*callbackOnRelease)(void))
{
    onTouch = callbackOnTouch;
    onRelease = callbackOnRelease;

    SysCtlPeripheralEnableAndReady(PERIPHERAL);
    GPIOPinTypeGPIOInput(PORT, PIN);
    GPIOIntTypeSet(PORT, PIN, GPIO_BOTH_EDGES);
    GPIOIntRegister(PORT, risingFallingEdgeIntHandler);
    GPIOIntEnable(PORT, PIN);
}

static void risingFallingEdgeIntHandler(void)
{
    GPIOIntClear(PORT, PIN);
    GPIOIntDisable(PORT, PIN);
    OS_SemaphoreSignal(&interruptHit);
}

void SwitchDebouncePB5_Task(void)
{
    int32_t lastPinValue = GPIOPinRead(PORT, PIN);
    while (1)
    {
        OS_SemaphoreWait(&interruptHit);
        lastPinValue ? onRelease() : onTouch();
        OS_ThreadSleep(10);
        lastPinValue = GPIOPinRead(PORT, PIN);
        GPIOIntEnable(PORT, PIN);
    }
}
