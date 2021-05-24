//*****************************************************************************
//
// Blink the LED on PA2, manipulating registers directly.
// For reference: https://users.ece.utexas.edu/~valvano/Volume1/E-Book/C8_SwitchLED.htm
//
//*****************************************************************************

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

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

static int main()
{
    volatile uint32_t ui32_loop;
    volatile unsigned long delay;

    SYSCTL_RCGCGPIO_R |= 0x01;        // 1) activate clock for Port A
    delay = SYSCTL_RCGCGPIO_R;        // allow time for clock to start
                                      // 2) no need to unlock PA2
    GPIO_PORTA_PCTL_R &= ~0x00000F00; // 3) regular GPIO
    GPIO_PORTA_AMSEL_R &= ~0x04;      // 4) disable analog function on PA2
    GPIO_PORTA_DIR_R |= 0x04;         // 5) set direction to output
    GPIO_PORTA_AFSEL_R &= ~0x04;      // 6) regular port function
    GPIO_PORTA_DEN_R |= 0x04;         // 7) enable digital port

    while (1)
    {
        // Make PA2 high
        GPIO_PORTA_DATA_R |= 0x04;

        // Delay for a bit
        for (ui32_loop = 0; ui32_loop < 1000000; ui32_loop++)
        {
        }

        // Make PA2 low
        GPIO_PORTA_DATA_R &= ~0x04;

        // Delay for a bit
        for (ui32_loop = 0; ui32_loop < 1000000; ui32_loop++)
        {
        }
    }
}
