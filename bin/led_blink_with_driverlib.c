//*****************************************************************************
//
// Blink the LED on PA2 using driverlib.
// Just a modified version of the original blinky.c
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

//
// The error routine that is called if the driver library encounters an error.
//
#ifdef DEBUG
static void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

static int main(void)
{
    volatile uint32_t ui32Loop;

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Check if the peripheral access is enabled.
    //
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }

    //
    // Enable the GPIO pin for the LED (PA2).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);

    //
    // Loop forever.
    //
    while (1)
    {
        //
        // Turn on the LED.
        //
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2);

        //
        // Delay for a bit.
        //
        for (ui32Loop = 0; ui32Loop < 1000000; ui32Loop++)
        {
        }

        //
        // Turn off the LED.
        //
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0x0);

        //
        // Delay for a bit.
        //
        for (ui32Loop = 0; ui32Loop < 1000000; ui32Loop++)
        {
        }
    }
}
