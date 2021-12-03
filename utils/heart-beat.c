#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "heart-beat.h"

static bool pinValue = false;

static void write(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, pinValue ? GPIO_PIN_2 : 0);
}

void HeartBeat_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
        ;
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
}

void HeartBeat_Set(void)
{
    pinValue = true;
    write();
}

void HeartBeat_Reset(void)
{
    pinValue = false;
    write();
}

void HeartBeat_Toggle(void)
{
    pinValue = !pinValue;
    write();
}

bool HeartBeat_GetPinValue(void)
{
    return pinValue;
}
