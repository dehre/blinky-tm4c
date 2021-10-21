//***************************************************************************************************
//
// Cycle through the different power modes available on the Tiva LaunchPad using the button on PC4.
// Three LEDs on PE012 are used to indicate the current power mode:
//     3 LEDs on - Run Mode
//     2 LEDs on - Sleep Mode
//     1 LED on  - Deep-Sleep Mode
// Timer0, in an ISR, toggles the LED on PE4 every 2 seconds.
//
// Check the document at 'bin/17_power_modes.md' for the difference between power modes.
// Circuit's diagram available in this repository.
// Date: 21-10-2021
//
//***************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

//
// Global variable to track the current power mode.
//
#define RUN_MODE 0
#define SLEEP_MODE 1
#define DEEP_SLEEP_MODE 2
static volatile uint32_t PowerMode = RUN_MODE;

//
// Timer0 generates periodic interrupts, at which we toggle the LED on PE4.
//
#define TIMER0_LOAD_VALUE (SysCtlClockGet() * 2) // trigger the ISR every 2 seconds
static void Timer0_Init(uint32_t timerLoadValue, void (*periodicIntHandler)(void));
static void Timer0_Enable(void);
static void Timer0_PeriodicIntHandler(void);

//
// PC4, configured as input, fires the interrupt handler on rising edges.
// The interrupt handler shifts to the next power mode and
//  updates the LEDs indicating the current power mode.
//
static void GPIO_PC4_Init(void);
static void GPIO_PC4_RisingEdgeIntHandler(void);

//
// PE012, configured as output, are updated (according to the global variable 'PowerMode')
//  each time the button on PC4 is pressed and drive the power mode indicator LEDs.
// PE4, configured as output, is toggled by Timer0 and drives the blinking LED.
//
static void GPIO_PE0124_Init(void);
static void GPIO_PE012_Set(void);
static void GPIO_PE4_Toggle(void);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    Timer0_Init(TIMER0_LOAD_VALUE, Timer0_PeriodicIntHandler);
    GPIO_PC4_Init();
    GPIO_PE0124_Init();

    GPIO_PE012_Set(); // adjust the LEDs according to the initial power mode.
    Timer0_Enable();

    //
    // Configure peripherals' behaviour on sleep and deep-sleep modes.
    // The peripheral isn't clocked, if not explicitly enabled.
    // If the GPIOE peripheral isn't clocked, the LEDs remain powered up,
    //  but pins' state cannot be changed.
    //  In this case, enabling the clock on LEDs' peripheral wouldn't have any effect:
    //  when the button on PC4 is pushed, the microcontroller returns to RUN_MODE anyway.
    //
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralDeepSleepEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralDeepSleepEnable(SYSCTL_PERIPH_GPIOC);

    // Enable Auto Clock Gating Control:
    //  the above settings wouldn't have any effect otherwise,
    //  and all peripherals would remain enabled (clocked) as if they were in RUN_MODE.
    SysCtlPeripheralClockGating(true);

    //
    // Switch to the internal oscillator and power down the main oscillator
    //  while in deep-sleep mode.
    //
    SysCtlDeepSleepClockSet(SYSCTL_DSLP_DIV_1 | SYSCTL_DSLP_OSC_INT | SYSCTL_DSLP_MOSC_PD);

    //
    // Set SRAM to Standby when in Sleep Mode.
    // Set Flash & SRAM to Low Power in Deep-Sleep Mode.
    // CCS fails to flash the program with one of these configuration active,
    //  so restart the microcontroller before doing that.
    //
    SysCtlSleepPowerSet(SYSCTL_SRAM_STANDBY);
    SysCtlDeepSleepPowerSet(SYSCTL_FLASH_LOW_POWER | SYSCTL_SRAM_LOW_POWER);

    //
    // After each time an interrupt wakes up the processor and the ISR completes,
    //  this while-loop is run next.
    //
    while (1)
    {
        if (PowerMode == SLEEP_MODE)
            SysCtlSleep();
        else if (PowerMode == DEEP_SLEEP_MODE)
            SysCtlDeepSleep();
        // else
        //     keep looping in RUN_MODE
    }
}

void Timer0_Init(uint32_t timerLoadValue, void (*periodicIntHandler)(void))
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
        ;
    // Full-width 32-bits periodic timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoadValue);
    TimerIntRegister(TIMER0_BASE, TIMER_A, periodicIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer0_Enable(void)
{
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer0_PeriodicIntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    GPIO_PE4_Toggle();
}

void GPIO_PC4_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTC_BASE, GPIO_PC4_RisingEdgeIntHandler);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
}

void GPIO_PC4_RisingEdgeIntHandler(void)
{
    SysCtlDelay(SysCtlClockGet() / 10); // simple debouncing
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
    PowerMode = (PowerMode + 1) % 3;
    GPIO_PE012_Set();
}

void GPIO_PE0124_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        ;
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4);
}

void GPIO_PE012_Set(void)
{
    switch (PowerMode)
    {
    case RUN_MODE:
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_PIN_1);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_PIN_2);
        break;
    case SLEEP_MODE:
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_PIN_1);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
        break;
    case DEEP_SLEEP_MODE:
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
        break;
    default:
        break;
    }
}

void GPIO_PE4_Toggle(void)
{
    static uint32_t current = 0;
    current ^= GPIO_PIN_4;
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, current);
}
