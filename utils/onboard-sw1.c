#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#include "onboard-sw1.h"

#define ONBOARD_SW1_PERIPH SYSCTL_PERIPH_GPIOF
#define ONBOARD_SW1_PORT GPIO_PORTF_BASE
#define ONBOARD_SW1_PIN GPIO_PIN_4

void OnboardSw1_Init(void)
{
    SysCtlPeripheralEnable(ONBOARD_SW1_PERIPH);
    while (!SysCtlPeripheralReady(ONBOARD_SW1_PERIPH))
        ;
    GPIOPinTypeGPIOInput(ONBOARD_SW1_PORT, ONBOARD_SW1_PIN);
    GPIOPadConfigSet(ONBOARD_SW1_PORT, ONBOARD_SW1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

uint32_t OnboardSw1_Read(void)
{
    uint32_t pinRead = GPIOPinRead(ONBOARD_SW1_PORT, ONBOARD_SW1_PIN);
    pinRead = (~pinRead) >> 4;
    return pinRead & 0x01;
}
