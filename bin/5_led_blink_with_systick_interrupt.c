//*****************************************************************************
//
// Blink the onboard LED on PF3 using the SysTick interrupt handler.
// Frequency 1 Hz
// Date: 20-06-2021
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

//
// The error routine that is called if the driver library encounters an error.
//
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

static void sysTickInterruptHandler(void);
static uint8_t toggleBits(uint8_t bits);

int main(void)
{
    //
    // Set the clocking to run directly from the crystal at 80MHz.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Check if the peripheral access is enabled.
    //
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    //
    // Enable the GPIO pin for the LED (PF3). Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    //
    // Configure and enable the SysTick counter for 100ms.
    //
    SysTickPeriodSet(8000000);
    SysTickIntRegister(sysTickInterruptHandler);
    SysTickEnable();

    //
    // Loop forever.
    //
    while (1)
    {
    }
}

void sysTickInterruptHandler(void)
{
    static uint8_t current = 0;
    uint8_t TARGET = 5;
    if (current < TARGET)
    {
        current++;
        return;
    }

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, toggleBits(GPIO_PIN_3));
    current = 0;
}

uint8_t toggleBits(uint8_t bits)
{
    static uint8_t current = 0;
    current ^= bits;
    return current;
}
