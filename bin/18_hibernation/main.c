//***************************************************************************************************
//
// Enter Hibernate Mode on user's input:
//  press and release SW1, then press and release SW1 & SW2 together.
// Hibernation can be exited by pushing SW2, or by waiting 30 seconds for the RTC timer.
//
// Useful link: https://unboxnbeyond.wordpress.com/2013/11/17/stellaris-launchpad-hibernation-module
//
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 451.
// Check the document at 'bin/17_power_modes.md' for the difference between power modes.
// Date: 25-10-2021
//
//***************************************************************************************************

//***************************************************************************************************
// OTHER IMPORTANT INFORMATION
//***************************************************************************************************
//
// To use SW2 as wake up source, unlock PF0 and turn on the internal WPU resistor.
// To unlock a GPIO pin: https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/374640/diagnosing-common-development-problems-and-tips-info-for-tm4c-devices
//
// If power is removed from the board during hibernation, nothing bad happens at restart:
//  the microcontroller simply forgets its previous setup and restarts from instruction zero.
//  Issues happen only if the microcontroller goes directly into hibernation after booting up,
//  without waiting for any user input or delay.
//
// From: https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/388799/external-rst-isnt-waking-device-from-hibernation-mode-tm4c123g7h6pgei
//  On the TM4C123 the RST pin is not a valid source of wakeup and doesn't have any effect during hibernation.
//  This feature is only for the TM4C129 device.
//
// From the microcontroller's data sheet page 496:
//  If a crystal is used for the clock source, the software must leave a delay of tHIBOSC_START after writing
//  to the CLK32EN bit (that is, after calling `HibernateEnableExpClk`) and before any other accesses to the
//  Hibernation module registers. The delay allows the crystal to power up and stabilize.
//  If an external oscillator is used for the clock source, no delay is needed.
//
//***************************************************************************************************

#include "stdbool.h"
#include "stdint.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/hibernate.h"
#include "onboard-leds.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

int main(void)
{
    uint32_t hibernateCount = 0;

    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_SYSDIV_1 | SYSCTL_XTAL_16MHZ);
    OnboardLEDs_Init(SysCtlClockGet() / 4);
    UART_Init();

    // Unlock PF_0
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= GPIO_PIN_0;

    // Configure switches SW1 and SW2.
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // If the hibernation module is already active, the processor is waking from a hibernation.
    if (HibernateIsActive())
    {
        uint32_t interruptStatus = HibernateIntStatus(false);
        HibernateIntClear(interruptStatus);
        HibernateDataGet(&hibernateCount, 1);

        if (interruptStatus & HIBERNATE_INT_PIN_WAKE)
            UARTprintf("Wake due to SW2, hibernateCount: %u\n", hibernateCount);
        else if (interruptStatus & HIBERNATE_INT_RTC_MATCH_0)
            UARTprintf("Wake due to RTC, hibernateCount: %u\n", hibernateCount);
        else
            UARTprintf("Wake due to hard reset, hibernateCount: %u\n", hibernateCount);
    }

    // Wait until SW1 is pressed and released.
    UARTprintf("Application started, waiting for SW1\n");
    while (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
        OnboardLEDs_RedBlink();
    while (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
        ;

    // Enable the hibernate module.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    HibernateEnableExpClk(SysCtlClockGet());

    // Enable the RTC and set the match registers to 30 seconds in the future.
    // The RTC starts counting from now, no matter when `HibernateRequest` will be issued.
    HibernateRTCEnable();
    HibernateRTCMatchSet(0, 30);
    HibernateRTCSet(0);

    // Set the wake condition on wake-pin or RTC match.
    HibernateWakeSet(HIBERNATE_WAKE_PIN | HIBERNATE_WAKE_RTC);

    UARTprintf("Hibernation setup done, waiting for SW1 and SW2\n");
    while (1)
    {
        OnboardLEDs_GreenBlink();

        // Check whether both SW1 and SW2 are pressed.
        if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0))
        {
            // Loop until both SW1 and SW2 are released.
            while (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0) != (GPIO_PIN_4 | GPIO_PIN_0))
                ;

            hibernateCount++;
            HibernateDataSet(&hibernateCount, 1);
            UARTprintf("Entering hibernation\n");
            OnboardLEDs_BlueBlink();

            HibernateGPIORetentionEnable();
            HibernateRequest();
            while (1)
                ;
        }
    }
}
