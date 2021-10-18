//***************************************************************************************************
//
// Vectored interrupts: detect rising edges on PC4 and PE4.
//
// There is a single interrupt service routine for each GPIO port.
// If two switches use different GPIO ports, the ISR knows which trigger caused the interrupt.
//
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 273.
// Circuit's diagram available in this repository.
// Date: 18-10-2021
//
//***************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "heart-beat.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

static void GPIO_PC4PE4_Init(void);
static void GPIO_PC4_risingEdgeIntHandler(void);
static void GPIO_PE4_risingEdgeIntHandler(void);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_INT |
                   SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();
    GPIO_PC4PE4_Init();
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_INT_PIN_4);

    while (1)
        SysCtlSleep();
}

void GPIO_PC4PE4_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTC_BASE, GPIO_PC4_risingEdgeIntHandler);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTE_BASE, GPIO_PE4_risingEdgeIntHandler);
}

void GPIO_PC4_risingEdgeIntHandler(void)
{
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
    HeartBeat_Set();
}

void GPIO_PE4_risingEdgeIntHandler(void)
{
    GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_4);
    HeartBeat_Reset();
}
