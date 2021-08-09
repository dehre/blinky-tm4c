#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

void heartBeatInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
        ;
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
}

void heartBeatSet(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
}

void heartBeatReset(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
}

static uint32_t toggleBits(uint32_t bits)
{
    static uint32_t current = 0;
    current ^= bits;
    return current;
}

void heartBeatToggle(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, toggleBits(GPIO_PIN_2));
}
