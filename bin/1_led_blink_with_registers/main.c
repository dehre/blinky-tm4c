//*****************************************************************************
//
// Blink the LED on PA2, manipulating registers directly.
// For reference: https://users.ece.utexas.edu/~valvano/Volume1/E-Book/C8_SwitchLED.htm
// Date: 24-05-2021
//
//*****************************************************************************

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

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

int main(void)
{
    volatile uint32_t ui32_loop;

    SYSCTL_RCGCGPIO_R |= 0x01;              // 1) activate clock for Port A         -- GPIO Run Mode Clock Gating Control Register
    while ((SYSCTL_PRGPIO_R & 0x01) == 0)   // allow time for clock to start        -- GPIO Peripheral Ready Register
    {
    }

                                            // 2) no need to unlock PA2             -- GPIO Lock Register
    GPIO_PORTA_AMSEL_R &= ~0x04;            // 3) disable analog function on PA2    -- GPIO Analog Mode Select Register
    GPIO_PORTA_PCTL_R &= ~0x00000F00;       // 4) regular GPIO                      -- GPIO Port Control Register
    GPIO_PORTA_DIR_R |= 0x04;               // 5) set direction to output           -- GPIO Direction Register
    GPIO_PORTA_AFSEL_R &= ~0x04;            // 6) regular port function             -- GPIO Alternate Function Select Register
    GPIO_PORTA_DEN_R |= 0x04;               // 7) enable digital port               -- GPIO Digital Enable Register

    while (1)
    {
        // make PA2 high
        GPIO_PORTA_DATA_R |= 0x04;

        // delay for a bit
        for (ui32_loop = 0; ui32_loop < 1000000; ui32_loop++)
        {
        }

        // make PA2 low
        GPIO_PORTA_DATA_R &= ~0x04;

        // delay for a bit
        for (ui32_loop = 0; ui32_loop < 1000000; ui32_loop++)
        {
        }
    }
}
