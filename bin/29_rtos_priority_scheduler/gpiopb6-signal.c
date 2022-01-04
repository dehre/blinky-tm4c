#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include "macro-utils.h"
#include "os.h"

#include "gpiopb6-signal.h"

#define PERIPHERAL SYSCTL_PERIPH_GPIOB
#define PORT GPIO_PORTB_BASE
#define PIN GPIO_PIN_6

int32_t GPIOPB6_Signal_RisingEdgeHit = 0;

static void risingEdgeIntHandler(void);

void GPIOPB6_Signal_Init(void)
{
    SysCtlPeripheralEnableAndReady(PERIPHERAL);
    GPIOPinTypeGPIOInput(PORT, PIN);
    GPIOIntTypeSet(PORT, PIN, GPIO_RISING_EDGE);
    GPIOIntRegister(PORT, risingEdgeIntHandler);
    GPIOIntEnable(PORT, PIN);
}

static void risingEdgeIntHandler(void)
{
    GPIOIntClear(PORT, PIN);
    OS_SemaphoreSignal(&GPIOPB6_Signal_RisingEdgeHit);
    OS_ThreadSuspend();
}
