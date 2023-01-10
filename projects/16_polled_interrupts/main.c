//***************************************************************************************************
//
// Polled interrupts: detect rising edges on PE4 and PE5.
//
// There is a single interrupt service routine for each GPIO port.
// If two switches use the same GPIO port, the ISR must determine which trigger generated the interrupt.
// The GPIOMIS register provides the status of interrupt caused by each pin:
//  the first eight bits of this register correspond to the PIN0 to PIN7 of each GPIO interrupt status.
// Further info on page 669 in the microcontroller's data sheet
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

static void GPIO_PE4PE5_Init(void);
static void GPIO_PE4PE5_risingEdgeIntHandler(void);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_INT |
                   SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();
    GPIO_PE4PE5_Init();
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_INT_PIN_4 | GPIO_INT_PIN_5);

    while (1)
        SysCtlSleep();
}

void GPIO_PE4PE5_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_INT_PIN_5, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTE_BASE, GPIO_PE4PE5_risingEdgeIntHandler);
}

void GPIO_PE4PE5_risingEdgeIntHandler(void)
{
    uint32_t GPIOMaskedInterruptStatus = GPIOIntStatus(GPIO_PORTE_BASE, true);
    if (GPIOMaskedInterruptStatus & GPIO_PIN_4)
    {
        GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_4);
        HeartBeat_Set();
    }
    if (GPIOMaskedInterruptStatus & GPIO_PIN_5)
    {
        GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_5);
        HeartBeat_Reset();
    }
}
