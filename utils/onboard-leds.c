#include "stdbool.h"
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"

static uint32_t delay;

static inline void blink(uint32_t gpioPin);

void OnboardLEDs_Init(uint32_t blinkDelayCount)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);

    ASSERT(blinkDelayCount >= 3);
    delay = blinkDelayCount;
}

void OnboardLEDs_RedBlink(void)
{
    blink(GPIO_PIN_1);
}

void OnboardLEDs_BlueBlink(void)
{
    blink(GPIO_PIN_2);
}

void OnboardLEDs_GreenBlink(void)
{
    blink(GPIO_PIN_3);
}

void blink(uint32_t gpioPin)
{
    GPIOPinWrite(GPIO_PORTF_BASE, gpioPin, 0xFF);
    SysCtlDelay(delay / 3);
    GPIOPinWrite(GPIO_PORTF_BASE, gpioPin, 0x00);
    SysCtlDelay(delay / 3);
}
