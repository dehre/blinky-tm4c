//********************************************************************************************
//
// Register interrupt statically (or dynamically) on the onboard switch SW1 on PF4.
// Let the processor enter sleep mode when not busy handling the interrupts.
// Date: 05-09-2021
//
//********************************************************************************************

//********************************************************************************************
//
// From the TivaWare™ Peripheral Driver Library:
// Interrupt handlers can be configured in one of two ways; statically at compile time or dynamically at run time.
// Static configuration of interrupt handlers is accomplished by editing the interrupt handler
// table in the application’s startup code. When statically configured, the interrupts must be explicitly
// enabled in the NVIC via IntEnable() before the processor can respond to the interrupt (in addition to
// any interrupt enabling required within the peripheral itself). Statically configuring the interrupt table
// provides the fastest interrupt response time because the stacking operation (a write to SRAM) can
// be performed in parallel with the interrupt handler table fetch (a read from Flash), as well as the
// prefetch of the interrupt handler itself (assuming it is also in Flash).
// Alternatively, interrupts can be configured at run-time using IntRegister() (or the analog in each
// individual driver). When using IntRegister(), the interrupt must also be enabled as before; when
// using the analogue in each individual driver, IntEnable() is called by the driver and does not need
// to be called by the application. Run-time configuration of interrupts adds a small latency to the
// interrupt response time because the stacking operation (a write to SRAM) and the interrupt handler
// table fetch (a read from SRAM) must be performed sequentially.
//
//********************************************************************************************

//********************************************************************************************
//
// From the Tiva™ TM4C123GH6PM Microcontroller Data Sheet:
// The Cortex-M4F processor sleep modes reduce power consumption:
// - Sleep mode stops the processor clock.
// - Deep-sleep mode stops the system clock and switches off the PLL and Flash memory.
// Interesting link: https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/455409/deep-sleep-sleep-modes
//
//********************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
#include "heart-beat.h"
#include "uart-init.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

static void OnboardSw1_InterruptHandler(void);

static void registerInterruptStatically(void);

static void registerInterruptDynamically(void);

static int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    UART_Init();
    HeartBeat_Init();
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // registerInterruptStatically();
    registerInterruptDynamically();

    UARTprintf("Application started\n");
    while (1)
    {
        UARTprintf("Going to sleep\n");
        SysCtlSleep();
    }
}

void OnboardSw1_InterruptHandler(void)
{
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    UARTprintf("Handling interrupt\n");
    HeartBeat_Toggle();
}

// To register the interrupt statically, replace `IntDefaultHandler`
// with `OnboardSw1_InterruptHandler` in the interrupt handler
// table `g_pfnVectors` in startup_ccs.c
void registerInterruptStatically(void)
{
    // `GPIOIntEnable()` makes the pin interrupt capable inside the GPIO module,
    // but then `IntEnable()` allows that particular GPIO port's interrupts to
    // actually raise the CPU's interrupt attention.
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    IntEnable(INT_GPIOF_TM4C123);
}

// To register the interrupt dynamically, there's no need to edit
// the interrupt handler table `g_pfnVectors` in startup_ccs.c
void registerInterruptDynamically(void)
{
    // The use of `GPIOIntRegister()` moves the interrupt vector table from flash to SRAM.
    // `GPIOIntRegister()` also enables the corresponding GPIO interrupt in the interrupt
    // controller, so calling `IntEnable()` isn't necessary.
    // Individual pin interrupts and interrupt sources must be enabled with `GPIOIntEnable()`.
    GPIOIntRegister(GPIO_PORTF_BASE, OnboardSw1_InterruptHandler);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
}
