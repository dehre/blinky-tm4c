//*****************************************************************************
//
// Output the sequence 10 9 5 6 on pins PD0-3.
// Delays implemented with Systick.
// For reference: book "Introduction to ARM Cortex-M Microcontrollers" page 175
// Date: 26-05-2021
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define GPIO_PIN_0_1_2_3 (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)

#define DELAY 50 // 500ms

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

static void SysTick_Init(void);

static void SysTick_Wait(uint32_t delay);

static void SysTick_Wait_10ms(uint32_t delay);

static int main(void)
{
    //
    // Set the clocking to run directly from the crystal at 80MHz.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //
    // Enable the SysTick counter.
    //
    SysTick_Init();

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    //
    // Check if the peripheral access is enabled.
    //
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }

    //
    // Enable the GPIO pin for the LEDs. Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3);

    //
    // Initial output
    //
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3, 0xF);

    //
    // Loop forever.
    //
    while (1)
    {
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3, 10);
        SysTick_Wait_10ms(DELAY);

        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3, 9);
        SysTick_Wait_10ms(DELAY);

        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3, 5);
        SysTick_Wait_10ms(DELAY);

        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3, 6);
        SysTick_Wait_10ms(DELAY);
    }
}

static void SysTick_Init(void)
{
    NVIC_ST_CTRL_R = 0;
    NVIC_ST_RELOAD_R = 0x00FFFFFF;
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R = 0x05;
}

static void SysTick_Wait(uint32_t delay)
{
    NVIC_ST_RELOAD_R = delay - 1;
    NVIC_ST_CURRENT_R = 0;
    while ((NVIC_ST_CTRL_R & 0x00010000) == 0)
    {
    }
}

static void SysTick_Wait_10ms(uint32_t delay)
{
    uint32_t i;
    for (i = 0; i < delay; i++)
    {
        SysTick_Wait(800000);
    }
}
