//***************************************************************************************************
//
// Measure the time between rising edge (PB5) and falling edge (PB6) of a pulse, in milliseconds.
// The result is transmitted to UART0, testable by using a serial terminal program on a host computer.
//
// In short:
// Timer0 is split into 2 half-width timers, synchronized with each other.
// TimerA starts on the rising edge (button pressed) and stops on the falling edge (button released).
// TimerB counts the number of times it timeouts, allowing longer time-measurements.
// So:
// numberOfClockCyclesElapsed = (numberOfTimeouts * loadValueTimerB) + currentValueTimerA
// timeElapsed = numberOfClockCyclesElapsed * durationOfAClockCycle
//
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 304.
// Circuit's diagram available in this repository.
// Date: 07-10-2021
//
//***************************************************************************************************

#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "heart-beat.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define HALF_WIDTH_TIMER_MAX_LOAD_VALUE 0xffff

//
// Get the duration of a clock cycle in nanoseconds.
// The value is memoized.
//
static uint32_t sysCtlClockGetNanoseconds(void);

//
// PB5 is configured as input and fires the interrupt handler on a rising edge.
// The interrupt handler starts Timer0.
//
static void GPIOPB5_Init(void (*risingEdgeIntHandler)(void));
static void GPIOPB5_RisingEdgeIntHandler(void);

//
// Timer0 is split into 2 half-width timers.
// TimerA, on PB6, fires the interrupt handler on a falling edge.
// TimerB fires the interrupt handler when it timeouts, that is, when its count reaches zero.
// The timeout interrupt handler increments the number of times TimerB restarts.
// The fallingEdge interrupt handler stops both Timers and computes the time elapsed between rising and falling edge.
//  The time elapsed is transmitted to the serial terminal over UART0.
//
static void Timer0PB6_Init(void (*fallingEdgeIntHandler)(void), void timeoutIntHandler(void));
static void Timer0PB6_FallingEdgeIntHandler(void);
static void Timer0PB6_TimeoutIntHandler(void);
static uint32_t Timer0PB6_TimeoutsCount = 0;

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_2 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();
    UART_Init();

    GPIOPB5_Init(GPIOPB5_RisingEdgeIntHandler);
    Timer0PB6_Init(Timer0PB6_FallingEdgeIntHandler, Timer0PB6_TimeoutIntHandler);

    while (1)
        SysCtlSleep();
}

uint32_t sysCtlClockGetNanoseconds(void)
{
    static uint32_t nanoseconds = 0;
    if (!nanoseconds)
    {
        nanoseconds = ((float)1 / SysCtlClockGet()) * 1000000000;
    }
    return nanoseconds;
}

void GPIOPB5_Init(void (*risingEdgeIntHandler)(void))
{
    //
    // Enable peripherals
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
        ;

    //
    // Configure PB6 as input and interrupt on rising edge
    //
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5);
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTB_BASE, risingEdgeIntHandler);
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_5);
}

void Timer0PB6_Init(void (*fallingEdgeIntHandler)(void), void (*timeoutIntHandler)(void))
{
    //
    // Enable peripherals
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
        ;

    //
    // Configure GPIO pins
    //
    GPIOPinConfigure(GPIO_PB6_T0CCP0);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6);

    //
    // Configure timers A and B
    //
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerDisable(TIMER0_BASE, TIMER_B);
    TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_PERIODIC));
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);
    TimerLoadSet(TIMER0_BASE, TIMER_A, HALF_WIDTH_TIMER_MAX_LOAD_VALUE);
    TimerLoadSet(TIMER0_BASE, TIMER_B, HALF_WIDTH_TIMER_MAX_LOAD_VALUE);
    TimerMatchSet(TIMER0_BASE, TIMER_B, 0);
    TimerSynchronize(TIMER0_BASE, TIMER_0A_SYNC | TIMER_0B_SYNC);

    //
    // Configure interrupts
    //
    TimerIntRegister(TIMER0_BASE, TIMER_A, fallingEdgeIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);
    TimerIntRegister(TIMER0_BASE, TIMER_B, timeoutIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_MATCH);
}

void GPIOPB5_RisingEdgeIntHandler(void)
{
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_5);
    HeartBeat_Set();
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
}

void Timer0PB6_FallingEdgeIntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_CAPA_EVENT);
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    HeartBeat_Reset();

    uint32_t timerValue = TimerValueGet(TIMER0_BASE, TIMER_B);

    // The operations are performed on 64 bits, otherwise they overflow when the button is pressed for more than a few seconds.
    uint64_t clockCyclesElapsed = (uint64_t)HALF_WIDTH_TIMER_MAX_LOAD_VALUE * Timer0PB6_TimeoutsCount + timerValue;
    uint64_t timeElapsedMs = (clockCyclesElapsed * sysCtlClockGetNanoseconds()) / 1000000;

    // UARTprintf has trouble printing 64 bits integers, hence the type castings.
    UARTprintf("A clock cycle takes %d nanoseconds\n", sysCtlClockGetNanoseconds());
    UARTprintf("Timeouts count: %d -- Timer value: %d\n", Timer0PB6_TimeoutsCount, timerValue);
    UARTprintf("Total number of clock cycles elapsed: %d\n", (uint32_t)clockCyclesElapsed);
    UARTprintf("Total time elapsed in ms: %d\n\n", (uint32_t)timeElapsedMs);

    TimerLoadSet(TIMER0_BASE, TIMER_A, HALF_WIDTH_TIMER_MAX_LOAD_VALUE);
    TimerLoadSet(TIMER0_BASE, TIMER_B, HALF_WIDTH_TIMER_MAX_LOAD_VALUE);
    Timer0PB6_TimeoutsCount = 0;
}

void Timer0PB6_TimeoutIntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMB_MATCH);
    Timer0PB6_TimeoutsCount++;
}
