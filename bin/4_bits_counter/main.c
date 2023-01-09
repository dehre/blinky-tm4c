//*****************************************************************************
//
// Four-bits counter. Positive logic for input and output.
// Switch registered on touch, bounce removed with Systick delay.
// For reference: book "Introduction to ARM Cortex-M Microcontrollers" page 188 lab 4.6
// Circuit's diagram available in this repository.
// Date: 31-05-2021
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

static void GPIOPinRead_WaitForTouch(uint32_t ui32Port, uint8_t ui8Pins);

static uint8_t get_count();

static void SysTick_Init(void);

static void SysTick_Wait(uint32_t delay);

static void SysTick_Wait_10ms(uint32_t delay);

int main(void)
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
    // Wait for the peripheral access to be enabled.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }

    //
    // Configure PA5 as Input and PD0-3 as Output.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3);

    while (1)
    {
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0_1_2_3, get_count());
        GPIOPinRead_WaitForTouch(GPIO_PORTA_BASE, GPIO_PIN_5);
    }
}

void GPIOPinRead_WaitForTouch(uint32_t ui32Port, uint8_t ui8Pins)
{
    // Wait for release.
    while (GPIOPinRead(ui32Port, ui8Pins) != 0)
    {
    }

    SysTick_Wait_10ms(1);

    // Wait for touch.
    while (GPIOPinRead(ui32Port, ui8Pins) == 0)
    {
    }
}

static volatile uint8_t count = 15;

uint8_t get_count()
{
    count += 1;
    if (count > 15)
    {
        count = 0;
    }
    return count;
}

void SysTick_Init(void)
{
    NVIC_ST_CTRL_R = 0;
    NVIC_ST_RELOAD_R = 0x00FFFFFF;
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R = 0x05;
}

void SysTick_Wait(uint32_t delay)
{
    NVIC_ST_RELOAD_R = delay - 1;
    NVIC_ST_CURRENT_R = 0;
    while ((NVIC_ST_CTRL_R & 0x00010000) == 0)
    {
    }
}

void SysTick_Wait_10ms(uint32_t delay)
{
    uint32_t i;
    for (i = 0; i < delay; i++)
    {
        SysTick_Wait(800000);
    }
}
