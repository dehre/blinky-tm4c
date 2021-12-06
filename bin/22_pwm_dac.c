//***************************************************************************************************
//
// Output a 1kHz sine wave using a PWM DAC.
// In short, PWM is started at 50% duty cycle, and
//   periodically updated on Timer0 interrupt requests.
//
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 422.
// Circuit's diagram available in this repository.
// Date: 06-12-2021
//
//***************************************************************************************************

//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "macro-utils.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define SINE_WAVE_LEN 32
static uint16_t SineWave[SINE_WAVE_LEN] = {
    125, 143, 159, 175, 189, 200, 208, 213, 215, 213, 208, 200, 189, 175, 159,
    143, 125, 107, 91, 75, 61, 50, 42, 37, 35, 37, 42, 50, 61, 75, 91, 107};

//
// Assuming a 25MHz clock, the PWM output wave is initialized at 100kHz by:
//   * setting the PWM clock divider to 1
//   * setting the period of the PWM Generator to 250
//
static void PWMPB6_Init(void);
static void PWMPB6_Start(void);
static void PWMPB6_PulseWidthSet(uint32_t width);

//
// Timer0 generates periodic interrupts (1000 * SINE_WAVE_LEN) times per second,
//   at which the PWM duty cycle is updated.
//
#define TIMER0_LOAD_VALUE (SysCtlClockGet() / (1000 * SINE_WAVE_LEN))
static void Timer0_Init(void (*periodicIntHandler)(void));
static void Timer0_PeriodicIntHandler(void);
static void Timer0_Start(void);

int main(void)
{
    // Set the clock at 25MHz
    SysCtlClockSet(SYSCTL_SYSDIV_8 | SYSCTL_USE_PLL |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    PWMPB6_Init();
    Timer0_Init(Timer0_PeriodicIntHandler);

    PWMPB6_Start();
    Timer0_Start();

    while (1)
        SysCtlSleep();
}

void PWMPB6_Init(void)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Set the PWM Generator period and the initial duty cycle.
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 250);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 125);

    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}

void PWMPB6_Start(void)
{
    // Enable the PWM0 Bit0 (PB6) output signal.
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
}

void PWMPB6_PulseWidthSet(uint32_t width)
{
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, width);
}

void Timer0_Init(void (*periodicIntHandler)(void))
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, TIMER0_LOAD_VALUE);
    TimerIntRegister(TIMER0_BASE, TIMER_A, periodicIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer0_PeriodicIntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    static uint32_t sineWaveIdx = 0;
    sineWaveIdx = (sineWaveIdx + 1) % SINE_WAVE_LEN;
    PWMPB6_PulseWidthSet(SineWave[sineWaveIdx]);
}

void Timer0_Start(void)
{
    TimerEnable(TIMER0_BASE, TIMER_A);
}
