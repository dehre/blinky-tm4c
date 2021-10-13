//***************************************************************************************************
//
// Control the speed of a DC motor through PWM.
// Two switches allow to dynamically increase and decrease the duty cycle from 60% to 98%.
// In short:
//  PWM is started at 98% duty cycle on PB6.
//  The motor's direction is fixed, set by PC4-5.
//  When either of the switches on PC6-7 is pressed, the interrupt handler is fired and the duty cycle updated.
//
// Limitations:
//  The L298N Dual H-Bridge Motor Driver has an on-board 78M05 5V regulator that can be enabled or disabled through the "5V EN Jumper".
//  When the jumper is in place, the 5V regulator is enabled, supplying logic power supply from the motor power supply.
//  When the jumper is removed, the 5V regulator is disabled and you have to supply 5 Volts separately through the 5 Volt input terminal.
//  If the motor power supply is below 12V, you can put the jumper in place.
//  If the motor power supply is greater than 12V, you should remove the jumper to avoid the onboard 5V regulator from getting damaged.
//  In my PWM circuit:
//  I kept the jumper in place, thus the power supply cannot be greater than 12V.
//  There is, however, a voltage drop of about 2V in the L298N motor driver, so my 12V motor ends up being powered at most by 10V.
//  Useful link: https://lastminuteengineers.com/l298n-dc-stepper-driver-arduino-tutorial
//
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 311.
// Circuit's diagram available in this repository.
// Date: 13-10-2021
//
//***************************************************************************************************

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
#include "heart-beat.h"

#ifdef DEBUG
// The compiler optimizes fn `__error__` away, if the variable `errorCode` is removed.
static volatile uint32_t errorCode = 0;
void __error__(char *pcFilename, uint32_t ui32Line)
{
    errorCode = 1;
    while (1)
        ;
}
#endif

//
// PC4-5, configured as output, set the motor's direction.
//
static void GPIOPC45_Init(void);

//
// PC6-7, configured as input, fire the interrupt handlers on rising edges.
// The interrupt handlers increase/decrease the PWM duty cycle.
// Useful links explaining how to handle multiple interrupts on the same port:
// * https://microcontrollerslab.com/gpio-interrupts-tm4c123-tiva-launchpad-edge-level-triggered/
// * https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/333539/multiple-interrupts-in-one-port
//
static void GPIOPC67_Init(void);
static void dutyCycleUpdateIntHandler(void);

//
// PB6 is configured for use by the PWM peripheral.
// The duty cycle is initially set to 98%.
// PS: PD0 is connected to PB6 through the onboard R9 resistor, thus unusable. See https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/334509/disable-pwm-output-of-pb6-pb7-from-pd0-and-pd1
//
typedef struct
{
    uint32_t value;
} PWMPB6_DutyCycleType;
static void PWMPB6_Init(void);
static PWMPB6_DutyCycleType PWMPB6_DutyCycle = {.value = 98};
static void PWMPB6_DutyCycleIncrease(PWMPB6_DutyCycleType *dutyCycle);
static void PWMPB6_DutyCycleDecrease(PWMPB6_DutyCycleType *dutyCycle);

//
// The PWM duty-cycle should not be set lower than 2% or higher than 98%.
// This is a known limitation:
// * https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/657527/tm4c123gh6pm-pwm-duty-cycle-can-not-be-set-to-0-with-tivaware/2417089
// * https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/643899/ccs-tm4c123gh6pz-pwm-when-i-give-duty-cycle-0-i-get-output-like-i-m-giving-255/2373019
//
static void PWMDutyCycleSet(uint32_t ui32Base, uint32_t ui32PWMOut, uint32_t ui32Gen, uint32_t ui32DutyCycle);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();

    GPIOPC45_Init();
    GPIOPC67_Init();
    PWMPB6_Init();

    while (1)
        SysCtlSleep();
}

void GPIOPC45_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
        ;
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x0);
}

void GPIOPC67_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_RISING_EDGE);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTC_BASE, dutyCycleUpdateIntHandler);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_7);
}

void dutyCycleUpdateIntHandler(void)
{
    // There is a single interrupt service routine for each GPIO port.
    // The GPIOMIS register provides the status of interrupt caused by each pin;
    //  the first eight bits of this register correspond to the PIN0 to PIN7 of each GPIO interrupt status.
    // Further info on page 669 in the microcontroller's data sheet
    uint32_t GPIOMaskedInterruptStatus = GPIOIntStatus(GPIO_PORTC_BASE, true);

    if (GPIOMaskedInterruptStatus & GPIO_INT_PIN_6)
    {
        GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
        PWMPB6_DutyCycleIncrease(&PWMPB6_DutyCycle);
    }

    if (GPIOMaskedInterruptStatus & GPIO_INT_PIN_7)
    {
        GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_7);
        PWMPB6_DutyCycleDecrease(&PWMPB6_DutyCycle);
    }
}

void PWMPB6_Init(void)
{
    //
    // Set the PWM clock to systemClock/8.
    // A faster PWM clock doesn't provide enough torque to the motor at lower duty cycles.
    //
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (
        !SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0) |
        !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
        ;
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);

    //
    // Configure the PWM0 to count up/down without synchronization.
    // Generally, Count-Down mode is used for generating left- or right-aligned PWM signals,
    //  while the Count-Up/Down mode is used for generating center-aligned PWM signals.
    //
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    //
    // Set the PWM period to 250Hz.  To calculate the appropriate parameter
    // use the following equation: N = (1 / f) * SysClk.  Where N is the
    // function parameter, f is the desired frequency, and SysClk is the
    // system clock frequency.
    // In this case you get: (1 / 250Hz) * 16MHz = 64000 cycles.
    // Note that the maximum period you can set is 2^16.
    //
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 64000);

    //
    // Set PWM0 to a duty cycle of 98%.
    //
    PWMDutyCycleSet(PWM0_BASE, PWM_OUT_0, PWM_GEN_0, 98);

    //
    // Enable the PWM generator block.
    //
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    //
    // Enable the PWM0 Bit0 (PB6) output signal.
    //
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
}

void PWMPB6_DutyCycleIncrease(PWMPB6_DutyCycleType *dutyCycle)
{
    dutyCycle->value += 4;
    if (dutyCycle->value > 99)
    {
        dutyCycle->value = 99;
        HeartBeat_Set();
    }
    else
    {
        HeartBeat_Reset();
    }

    PWMDutyCycleSet(PWM0_BASE, PWM_OUT_0, PWM_GEN_0, dutyCycle->value);
}

void PWMPB6_DutyCycleDecrease(PWMPB6_DutyCycleType *dutyCycle)
{
    //
    // A duty cycle lower than 60% doesn't provide enough torque for the engine to move.
    //
    dutyCycle->value -= 4;
    if (dutyCycle->value < 60)
    {
        dutyCycle->value = 60;
        HeartBeat_Set();
    }
    else
    {
        HeartBeat_Reset();
    }

    PWMDutyCycleSet(PWM0_BASE, PWM_OUT_0, PWM_GEN_0, dutyCycle->value);
}

void PWMDutyCycleSet(uint32_t ui32Base, uint32_t ui32PWMOut, uint32_t ui32Gen, uint32_t ui32DutyCycle)
{
    ASSERT(ui32DutyCycle <= 100);
    if (ui32DutyCycle < 2)
        ui32DutyCycle = 2;
    else if (ui32DutyCycle > 98)
        ui32DutyCycle = 98;

    //
    // The duty cycle is set as a function of the period.
    // In this case, the PWM will be high for `ui32DutyCycle`% of the time.
    //
    uint32_t pulseWidth = (PWMGenPeriodGet(ui32Base, ui32Gen) * ui32DutyCycle) / 100;
    PWMPulseWidthSet(ui32Base, ui32PWMOut, pulseWidth);
}
