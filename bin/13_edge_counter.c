//***************************************************************************************************
//
// Count the number of positive edges detected on PB2 every second.
// Timer0 is used to generate a periodic interrupt, at which we read the number of positive edges.
// Timer3 is used to count the number of positive edges, and is reloaded each time its value is read.
// The result is transmitted to UART0, testable by using a serial terminal program on a host computer.
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 313.
// Circuit's diagram available in this repository.
// Date: 21-09-2021
//
//***************************************************************************************************

#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "heart-beat.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

//
// Timer0 generates periodic interrupts, at which we read the number of rising edges.
//
#define TIMER0_LOAD_VALUE (SysCtlClockGet() / 1) // trigger the interrupt once per second
static void Timer0_Init(uint32_t timerLoadValue, void (*periodicIntHandler)(void));
static void Timer0_PeriodicIntHandler(void);

//
// Timer3 counts the number of rising edges on PB2.
// On overflow, the interrupt handler toggles the onboard LED and restarts the timer.
// Timer3 is configured as 16-bit half-width timer, so the count for positive edges can be up to 0xffff.
//
#define TIMER3_MAX_EDGES_COUNT 0xffff
static void Timer3_Init(uint32_t maxEdgesCount, void (*overflowIntHandler)(void));
static void Timer3_OverflowIntHandler(void);

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

static int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();
    UART_Init();
    Timer0_Init(TIMER0_LOAD_VALUE, Timer0_PeriodicIntHandler); // periodic interrupt every second
    Timer3_Init(TIMER3_MAX_EDGES_COUNT, Timer3_OverflowIntHandler);

    TimerEnable(TIMER0_BASE, TIMER_A);
    TimerEnable(TIMER3_BASE, TIMER_A);

    while (1)
    {
        SysCtlSleep();
    }
}

void Timer0_Init(uint32_t timerLoadValue, void (*periodicIntHandler)(void))
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
        ;
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoadValue);
    TimerIntRegister(TIMER0_BASE, TIMER_A, periodicIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer3_Init(uint32_t maxEdgesCount, void (*overflowIntHandler)(void))
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
        ;

    GPIOPinConfigure(GPIO_PB2_T3CCP0);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);

    // `TIMER_CFG_A_CAP_COUNT` doesn't work without TIMER_CFG_SPLIT_PAIR: check data sheet p.708 and driver library user's guide p.539
    TimerDisable(TIMER3_BASE, TIMER_A);
    TimerConfigure(TIMER3_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT));
    TimerControlEvent(TIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
    TimerLoadSet(TIMER3_BASE, TIMER_A, maxEdgesCount);
    TimerMatchSet(TIMER3_BASE, TIMER_A, 0);

    // Dynamic interrupt on match.
    TimerIntRegister(TIMER3_BASE, TIMER_A, overflowIntHandler);
    TimerIntEnable(TIMER3_BASE, TIMER_CAPA_MATCH);

    // // Alternatively, static interrupt on match.
    // IntEnable(INT_TIMER3A);
    // TimerIntEnable(TIMER3_BASE, TIMER_CAPA_MATCH);
}

void Timer0_PeriodicIntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    uint32_t currentEdgesCount = TimerValueGet(TIMER3_BASE, TIMER_A);
    UARTprintf("Number of positive edges detected: %u\n", TIMER3_MAX_EDGES_COUNT - currentEdgesCount);
    TimerLoadSet(TIMER3_BASE, TIMER_A, TIMER3_MAX_EDGES_COUNT);
}

void Timer3_OverflowIntHandler(void)
{
    TimerIntClear(TIMER3_BASE, TIMER_CAPA_MATCH);
    HeartBeat_Toggle();
    // The timer is automatically stopped when it reaches the match value, so re-enable it here.
    TimerEnable(TIMER3_BASE, TIMER_A);
}
